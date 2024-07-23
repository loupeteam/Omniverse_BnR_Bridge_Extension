# This software contains source code provided by NVIDIA Corporation.
# Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#

import omni.ui as ui
import omni.timeline

from carb.settings import get_settings

from .websockets_driver import WebsocketsDriver, PLCDataParsingException, WebsocketsConnectionException

from .global_variables import EXTENSION_NAME
from .BrBridge import EVENT_TYPE_DATA_READ, EVENT_TYPE_DATA_READ_REQ, EVENT_TYPE_DATA_WRITE_REQ, EVENT_TYPE_DATA_INIT

import threading
from threading import RLock

import asyncio
import json
from websockets.exceptions import ConnectionClosed, ConnectionClosedError

import time

# Defaults / test variables for "Dev Tools" section of the UI
DEFAULT_DEV_TEST_UI_VAR = "TestProg:counter"
DEFAULT_DEV_TEST_UI_VALUE = "0"
DEFAULT_DEV_TEST_UI_WRITE_VAR = "TestProg:counterToggle"
DEFAULT_DEV_TEST_UI_WRITE_VALUE = "1"
# These are test variables in the sample AS program included with this repo
TEST_PROGRAM_VARS = ["TestProg:counter",
                     "TestProg:counter2", 
                     "TestProg:bool", 
                     "TestProg:int", 
                     "TestProg:dint", 
                     "TestProg:real", 
                     "TestProg:lreal", 
                     "TestProg:string"]
 
class UIBuilder:
    def __init__(self):
        # UI elements created using a UIElementWrapper instance
        self.wrapped_ui_elements = []

        # Get access to the timeline to control stop/pause/play programmatically
        self._timeline = omni.timeline.get_timeline_interface()

        # Get the settings interface
        self.settings_interface = get_settings()
         
        # Internal status flags. 
        self._thread_is_alive = True   
        self._communication_initialized = False
        self._ui_initialized = False

        # Configuration parameters for the extension.
        # These are exposed on the UI. 
        self._enable_communication = self.get_setting( 'ENABLE_COMMUNICATION', False ) 
        self._refresh_rate = self.get_setting( 'REFRESH_RATE', 20 ) # in ms

        # Timing variables
        self._actual_cyclic_read_time = 0
        self._last_cyclic_read_time = 0
        self._worst_latency = 0
        self._average_latency = 0

        # Data stream where the extension will dump the data that it reads from the PLC.
        self._event_stream = omni.kit.app.get_app().get_message_bus_event_stream()

        self._websockets_connector = WebsocketsDriver(ip=self.get_setting('PLC_IP_ADDRESS', '127.0.0.1'), port=self.get_setting('PLC_PORT', 8000))
        self._disconnect_command = False # command to trigger disconnect from outside async context
        
        self.write_queue = dict()
        self.write_lock = RLock()

        self.read_req = self._event_stream.create_subscription_to_push_by_type(EVENT_TYPE_DATA_READ_REQ, self.on_read_req_event)
        self.write_req = self._event_stream.create_subscription_to_push_by_type(EVENT_TYPE_DATA_WRITE_REQ, self.on_write_req_event)
        self._event_stream.push(event_type=EVENT_TYPE_DATA_INIT, payload={'data': {}})

        self._thread = threading.Thread(target=lambda: asyncio.run(self._update_plc_data()))
        self._thread.start()

    ###################################################################################
    #           The Functions Below Are Called Automatically By extension.py
    ###################################################################################

    def on_menu_callback(self):
        """Callback for when the UI is opened from the toolbar. 
        This is called directly after build_ui().
        """
        self._event_stream.push(event_type=EVENT_TYPE_DATA_INIT, payload={'data': {}})

        if not self._thread_is_alive:
            self._thread_is_alive = True
            self._thread.start()

    def on_timeline_event(self, event):
        """Callback for Timeline events (Play, Pause, Stop)

        Args:
            event (omni.timeline.TimelineEventType): Event Type
        """
        if(event.type == int(omni.timeline.TimelineEventType.STOP)):
            pass
        elif(event.type == int(omni.timeline.TimelineEventType.PLAY)):
            pass
        elif(event.type == int(omni.timeline.TimelineEventType.PAUSE)):
            pass     
   
    def on_stage_event(self, event):
        """Callback for Stage Events

        Args:
            event (omni.usd.StageEventType): Event Type
        """
        pass

    def cleanup(self):
        """
        Called when the stage is closed or the extension is hot reloaded.
        Perform any necessary cleanup such as removing active callback functions
        """
        self.read_req.unsubscribe()
        self.write_req.unsubscribe()
        self._thread_is_alive = False
        self._thread.join()

    def build_ui(self):
        """
        Build a custom UI tool to run your extension.  
        This function will be called any time the UI window is closed and reopened.
        """
        with ui.CollapsableFrame("Configuration", collapsed=False):
            with ui.VStack(spacing=5, height=0):

                with ui.HStack(spacing=5, height=0):
                    ui.Label("Enable Client")
                    self._enable_communication_checkbox = ui.CheckBox(ui.SimpleBoolModel(self._enable_communication))
                    self._enable_communication_checkbox.model.add_value_changed_fn(self._toggle_communication_enable)
                
                with ui.HStack(spacing=5, height=0):
                    ui.Label("Refresh Rate (ms)")
                    self._refresh_rate_field = ui.IntField(ui.SimpleIntModel(self._refresh_rate))
                    self._refresh_rate_field.model.set_min(10)
                    self._refresh_rate_field.model.set_max(10000)
                    self._refresh_rate_field.model.add_value_changed_fn(self._on_refresh_rate_changed)
                                   
                with ui.HStack(spacing=5, height=0):
                    ui.Label("PLC IP Address")
                    self._plc_ip_field = ui.StringField(ui.SimpleStringModel(self._websockets_connector.ip))
                    self._plc_ip_field.model.add_value_changed_fn(self._on_plc_ip_changed)

                with ui.HStack(spacing=5, height=0):
                    ui.Label("PLC Port")
                    self._plc_port_field = ui.IntField(ui.SimpleIntModel(self._websockets_connector.port))
                    self._plc_port_field.model.add_value_changed_fn(self._on_plc_port_changed)
                    self._plc_port_field.model.set_min(0)
                    self._plc_port_field.model.set_max(65535)

                with ui.HStack(spacing=5, height=0):
                    ui.Label("Settings")
                    ui.Button("Load", clicked_fn=self.load_settings)
                    ui.Button("Save", clicked_fn=self.save_settings)

        with ui.CollapsableFrame("Status", collapsed=False):
            with ui.VStack(spacing=5, height=0):
                with ui.HStack(spacing=5, height=0):
                    ui.Label("Status")
                    self._status_field = ui.StringField(ui.SimpleStringModel("n/a"), read_only=True)

        with ui.CollapsableFrame("Monitor", collapsed=False):
            with ui.VStack(spacing=5, height=0):
                with ui.HStack(spacing=5, height=100):
                    ui.Label("PLC Variables")
                    self._monitor_field = ui.StringField(ui.SimpleStringModel("{}"), multiline=True, read_only=True)

        with ui.CollapsableFrame("Dev Tools", collapsed=True):
            with ui.VStack(spacing=5, height=0):
                ui.Label("Average PLC read latency")
                self._average_cyclic_read_time_field = ui.FloatField(ui.SimpleFloatModel(self._average_latency), 
                                                                     multiline=False,
                                                                     read_only=True)
                ui.Label("Worst PLC read latency")
                self._worst_cyclic_read_time_field = ui.FloatField(ui.SimpleFloatModel(self._worst_latency), 
                                                                   multiline=False,
                                                                   read_only=True)
                self._test_read_button = ui.Button(text="Reset worst-case latency", 
                                                   clicked_fn=self._reset_worst_latency)
                ui.Label("Last PLC read latency")
                self._actual_cyclic_read_time_field = ui.FloatField(ui.SimpleFloatModel(self._actual_cyclic_read_time), 
                                                                    multiline=False, 
                                                                    read_only=True)

                self._separator = ui.Separator()

                self._test_read_button = ui.Button(text="Add variables for test program", 
                                                   clicked_fn=self._add_variables_for_test_program)
                self._test_read_field = ui.StringField(ui.SimpleStringModel(DEFAULT_DEV_TEST_UI_VAR), 
                                                       multiline=True,
                                                       read_only=False)
                self._test_read_button = ui.Button(text="Add Var To Cyclic Reads", 
                                                   clicked_fn=lambda: self._websockets_connector.add_read(plc_var=self._test_read_field.model.as_string))
                self._clear_read_list_button = ui.Button(text="Clear Read List", 
                                                         clicked_fn=self._websockets_connector.clear_read_list)

                self._separator = ui.Separator()
                
                self._test_write_field = ui.StringField(ui.SimpleStringModel(DEFAULT_DEV_TEST_UI_WRITE_VAR), 
                                                        multiline=True, 
                                                        read_only=False)
                self._test_write_field_value = ui.StringField(ui.SimpleStringModel(DEFAULT_DEV_TEST_UI_WRITE_VALUE), multiline=True, read_only=False)
                self._test_read_button = ui.Button(text="Write value",
                                                   clicked_fn=lambda: self.queue_write(name=self._test_write_field.model.as_string, value=self._test_write_field_value.model.as_string))
                
        self._ui_initialized = True

    def _add_variables_for_test_program(self):
        """
        Add a stock set of variables, corresponding to test variables in the sample AS program, to the readlist.
        """
        for var in TEST_PROGRAM_VARS:
            self._websockets_connector.add_read(plc_var=var)

    ####################################
    ####################################
    # UTILITY FUNCTIONS
    ####################################
    ####################################

    def on_read_req_event(self, event):
        """Callback for extension event stream. On read request event, add the variables to the read list."""
        event_data = event.payload
        variables : list = event_data['variables']
        for var in variables:
            self._websockets_connector.add_read(plc_var=var)

    def on_write_req_event(self, event):
        """Callback for extension event stream. On write request event, add a variable to the write queue."""
        variables = event.payload["variables"]
        for variable in variables:
            self.queue_write(variable['name'], variable['value'])

    def queue_write(self, name, value):
        """
        Add PLC variable to the write queue for sending variables and values to the PLC.
        
        Args
        name: str
            The name of the variable to write to.
        value:
            The value to write to the variable.
        """
        with self.write_lock:
            self.write_queue[name] = value

    def _update_ui_status(self, message, reset_monitor=False):
        """
        Update the status field with a message and optionally reset the monitor field.
        
        Args:
        message: str
            The message to display in the status field.
        reset_monitor: bool
            If True, the monitor field will be reset to an empty dict.
        """
        if self._ui_initialized:
            self._status_field.model.set_value(message)
            if reset_monitor:
                self._monitor_field.model.set_value("{}")

    def _update_monitor_field(self):
        """Update the variable-monitoring field in the UI."""
        if self._ui_initialized:
            json_formatted_str = json.dumps(self._data, indent=4)
            self._monitor_field.model.set_value(json_formatted_str)

    async def _update_plc_data(self):
        """
        Main loop for connecting, auto-reconnecting, reading data, and writing data to the PLC.
        """

        DATA_READ_FAIL_SLEEP_TIME_SECONDS = 2 # wait this long before retrying, also allows UI status to stick around

        thread_start_time = time.time()

        while self._thread_is_alive:

            # Sleep for the refresh rate minus time spent in last cycle
            last_cycle_time = time.time() - thread_start_time
            sleepy_time = self._refresh_rate/1000 - last_cycle_time
            if sleepy_time > 0:
                time.sleep(sleepy_time)

            thread_start_time = time.time()

            # Handle disconnect
            if self._disconnect_command:
                await self._websockets_connector.disconnect()
                self._disconnect_command = False
                continue

            # Check if the communication is disabled
            if not self._enable_communication:
                self._update_ui_status(message="Disabled", reset_monitor=True)
                continue

            # Start the communication if it is and not initialized and enabled 
            if not self._communication_initialized and self._enable_communication:
                # Attempt to connect
                self._update_ui_status("Attempting to connect...")
                try:
                    if await self._websockets_connector.connect():
                        self._communication_initialized = True
                        self._update_ui_status("Connected")
                        self._reset_worst_latency()
                except WebsocketsConnectionException as e:
                    self._update_ui_status(f"{e}")
                    time.sleep(DATA_READ_FAIL_SLEEP_TIME_SECONDS)
                    continue

            # Catch exceptions and log them to the status field
            try:
                if self._websockets_connector.is_connected():
                    try:
                        if self.write_queue:                                             
                            with self.write_lock:
                                # TODO would it be better if this was a deepcopy?
                                values = self.write_queue
                                self.write_queue = {}
                            await self._websockets_connector.write_data(values)

                    except ConnectionClosed as e:
                        self._update_ui_status(f"Connection Closed: {e}")

                    except Exception as e:
                        self._update_ui_status(f"Error writing data to PLC: {e}")

                    # Read data from the PLC
                    try:
                        self._data = await self._websockets_connector.read_data()
                    except PLCDataParsingException as e:
                        self._update_ui_status(f"PLC read data prasing error: {e}")

                    # Push the data to the event stream
                    self._event_stream.push(event_type=EVENT_TYPE_DATA_READ, payload={'data': self._data})

                    self._update_monitor_field()

                    self._calculate_statistics()

            except ConnectionClosedError as e:
                self._update_ui_status(f"Connection Closed: {e}")
                self._communication_initialized = False

            except Exception as e:
                self._update_ui_status(f"Error: {e}")
                time.sleep(DATA_READ_FAIL_SLEEP_TIME_SECONDS)

    ####################################
    ####################################
    # Statistics
    ####################################
    ####################################

    def _calculate_statistics(self):
        """
        Calculate timing statistics for the read / write cycle.
        """
        self._actual_cyclic_read_time = time.time() - self._last_cyclic_read_time
        
        self._actual_cyclic_read_time_field.model.set_value(self._actual_cyclic_read_time)

        self._average_latency = self.rolling_average(self._average_latency, self._actual_cyclic_read_time)
        self._average_cyclic_read_time_field.model.set_value(self._average_latency)

        if (self._actual_cyclic_read_time > self._worst_latency):
            self._worst_latency = self._actual_cyclic_read_time
            self._worst_cyclic_read_time_field.model.set_value(self._worst_latency)
        
        # Reset for next scan
        self._last_cyclic_read_time = time.time()

    def rolling_average(self, average, new):
        """Calculate a rolling average over a given number of samples."""
        NUM_SAMPLES = 10
        average -= average / NUM_SAMPLES
        average += new / NUM_SAMPLES
        return average
    
    def _reset_worst_latency(self):
        self._worst_latency = 0

    ####################################
    ####################################
    # Manage Settings
    ####################################
    ####################################

    def get_setting(self, name, default_value=None ):
        setting = self.settings_interface.get("/persistent/" + EXTENSION_NAME + "/" + name)
        if setting is None:
            setting = default_value
            self.settings_interface.set("/persistent/" + EXTENSION_NAME + "/" + name, setting)
        return setting

    def set_setting(self, name, value ):
        self.settings_interface.set("/persistent/" + EXTENSION_NAME + "/" + name, value)

    def _on_plc_ip_changed(self, value):
        self._websockets_connector.ip = value.get_value_as_string()
        self._communication_initialized = False
        if self._websockets_connector:
            self._disconnect_command = True

    def _on_plc_port_changed(self, value):
        self._websockets_connector.port = value.get_value_as_int()
        self._communication_initialized = False
        if self._websockets_connector:
            self._disconnect_command = True

    def _on_refresh_rate_changed(self, value):
        self._refresh_rate = value.get_value_as_int()

    def _toggle_communication_enable(self, state):
        self._enable_communication = state.get_value_as_bool()
        if not self._enable_communication:
            self._disconnect_command = True
            self._communication_initialized = False

    def save_settings(self):
        self.set_setting('REFRESH_RATE', self._refresh_rate)
        self.set_setting('PLC_IP_ADDRESS', self._websockets_connector.ip)
        self.set_setting('PLC_PORT', self._websockets_connector.port)
        self.set_setting('ENABLE_COMMUNICATION', self._enable_communication)

    def load_settings(self):
        self._refresh_rate = self.get_setting('REFRESH_RATE')
        self._websockets_connector.ip = self.get_setting('PLC_IP_ADDRESS')
        self._websockets_connector.port = self.get_setting('PLC_PORT')
        self._enable_communication = self.get_setting('ENABLE_COMMUNICATION')

        self._refresh_rate_field.model.set_value(self._refresh_rate)
        self._plc_ip_field.model.set_value(self._websockets_connector.ip)
        self._plc_port_field.model.set_value(self._websockets_connector.port)
        self._enable_communication_checkbox.model.set_value(self._enable_communication)
        self._communication_initialized = False
        if self._websockets_connector:
            self._disconnect_command = True


