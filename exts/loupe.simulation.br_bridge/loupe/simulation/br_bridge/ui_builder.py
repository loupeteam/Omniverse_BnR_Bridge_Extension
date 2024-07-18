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

from .websockets_driver import WebsocketsDriver, PLCDataParsingException

from .global_variables import EXTENSION_NAME
from .br_bridge import EVENT_TYPE_DATA_READ, EVENT_TYPE_DATA_READ_REQ, EVENT_TYPE_DATA_WRITE_REQ, EVENT_TYPE_DATA_INIT

import threading
from threading import RLock

import asyncio
import json
from websockets.exceptions import ConnectionClosed

import time
 
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
        self._refresh_rate = self.get_setting( 'REFRESH_RATE', 20 )

        # Data stream where the extension will dump the data that it reads from the PLC.
        self._event_stream = omni.kit.app.get_app().get_message_bus_event_stream()

        self._websockets_connector = WebsocketsDriver(ip=self.get_setting('PLC_IP_ADDRESS', '127.0.0.1'), port=self.get_setting('PLC_PORT', 8000))
        
        self.write_queue = dict()
        self.write_lock = RLock()

        self.read_req = self._event_stream.create_subscription_to_push_by_type(EVENT_TYPE_DATA_READ_REQ, self.on_read_req_event)
        self.write_req = self._event_stream.create_subscription_to_push_by_type(EVENT_TYPE_DATA_WRITE_REQ, self.on_write_req_event)
        self._event_stream.push(event_type=EVENT_TYPE_DATA_INIT, payload={'data': {}})

        self._thread = threading.Thread(target=lambda: asyncio.run(self._update_plc_data()))
        self._thread.start()

        self._actual_cyclic_read_time = 0
        self._last_cyclic_read_time = 0

    ###################################################################################
    #           The Functions Below Are Called Automatically By extension.py
    ###################################################################################

    def on_menu_callback(self):
        """Callback for when the UI is opened from the toolbar. 
        This is called directly after build_ui().
        """
        self._event_stream.push(event_type=EVENT_TYPE_DATA_INIT, payload={'data': {}})

        if(not self._thread_is_alive):
            self._thread_is_alive = True
            # TODO confirm this redefinition is not needed: self._thread = threading.Thread(target=self._update_plc_data)
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
                self._actual_cyclic_read_time_field = ui.FloatField(ui.SimpleFloatModel(self._actual_cyclic_read_time), multiline=False, read_only=True)
                self._test_read_button = ui.Button(text="Add variables for test program", 
                                                   clicked_fn=self._add_variables_for_test_program)
                self._test_read_field = ui.StringField(ui.SimpleStringModel("LuxProg:counter"), multiline=True, read_only=False) # TODO remove test var
                self._test_read_button = ui.Button(text="Add Var To Cyclic Reads", 
                                                   clicked_fn=lambda: self._websockets_connector.add_read(name=self._test_read_field.model.as_string))
                self._clear_read_list_button = ui.Button(text="Clear Read List", 
                                                         clicked_fn=self._websockets_connector.clear_read_list) # TODO cleanup

                self._separator = ui.Separator()
                
                self._test_write_field = ui.StringField(ui.SimpleStringModel("LuxProg:counter"), multiline=True, read_only=False) # TODO remove test var
                self._test_write_field_value = ui.StringField(ui.SimpleStringModel("1000"), multiline=True, read_only=False) # TODO remove test var
                self._test_read_button = ui.Button(text="Write value",
                                                   clicked_fn=lambda: self.queue_write(name=self._test_write_field.model.as_string, value=self._test_write_field_value.model.as_string))


        self._ui_initialized = True

    def _add_variables_for_test_program(self):
        self._websockets_connector.add_read("LuxProg:counter")
        self._websockets_connector.add_read("LuxProg:counter2")
        self._websockets_connector.add_read("LuxProg:bool")
        self._websockets_connector.add_read("LuxProg:int")
        self._websockets_connector.add_read("LuxProg:dint")
        self._websockets_connector.add_read("LuxProg:real")
        self._websockets_connector.add_read("LuxProg:lreal")
        self._websockets_connector.add_read("LuxProg:string")

    ####################################
    ####################################
    # UTILITY FUNCTIONS
    ####################################
    ####################################

    def on_read_req_event(self, event ):
        event_data = event.payload
        variables : list = event_data['variables'] 
        for name in variables:
            self._websockets_connector.add_read(name)

    def on_write_req_event(self, event ):
        variables = event.payload["variables"]
        for variable in variables:
            self.queue_write(variable['name'], variable['value'])

    def queue_write(self, name, value):
        with self.write_lock:
            self.write_queue[name] = value

    async def _update_plc_data(self):

        thread_start_time = time.time()
        status_update_time = time.time()

        while self._thread_is_alive:

            # Sleep for the refresh rate
            sleepy_time = self._refresh_rate/1000 - (time.time() - thread_start_time)
            if sleepy_time > 0:
                time.sleep(sleepy_time)

            thread_start_time = time.time()

            # Check if the communication is enabled
            if not self._enable_communication:
                if self._ui_initialized:
                    self._status_field.model.set_value("Disabled")
                    self._monitor_field.model.set_value("{}")
                continue

            # Catch exceptions and log them to the status field
            try:
                # Start the communication if it is not initialized
                if (not self._communication_initialized) and (self._enable_communication):
                    await self._websockets_connector.connect()
                    self._communication_initialized = True
                elif (self._communication_initialized) and (not self._websockets_connector.is_connected()):
                    await self._websockets_connector.disconnect()
                    self._communication_initialized = False

                if status_update_time < time.time():
                    if self._websockets_connector.is_connected():
                        self._status_field.model.set_value("Connected")
                    else:
                        self._status_field.model.set_value("Attempting to connect...")

                # Write data to the PLC if there is data to write
                # If there is an exception, log it to the status field but continue reading data
                try:
                    if self.write_queue:                                             
                        with self.write_lock:
                            # TODO would it be better if this was a deepcopy?
                            values = self.write_queue
                            self.write_queue = dict()
                        await self._websockets_connector.write_data(values)

                except ConnectionClosed as e:
                    if self._ui_initialized:
                        self._status_field.model.set_value(f"Connection Closed: {e}")
                        # TODO disconnect?
                        status_update_time = time.time() + 1

                except Exception as e:
                    if self._ui_initialized:
                        self._status_field.model.set_value(f"Error writing data to PLC: {e}")
                        status_update_time = time.time() + 1

                # Read data from the PLC
                self._data = await self._websockets_connector.read_data()
                self._actual_cyclic_read_time = time.time() - self._last_cyclic_read_time
                self._last_cyclic_read_time = time.time()
                self._actual_cyclic_read_time_field.model.set_value(self._actual_cyclic_read_time)

                # Push the data to the event stream
                self._event_stream.push(event_type=EVENT_TYPE_DATA_READ, payload={'data': self._data})

                # Update the monitor field
                if self._ui_initialized:
                    json_formatted_str = json.dumps(self._data, indent=4)
                    self._monitor_field.model.set_value(json_formatted_str)

            except Exception as e:
                if self._ui_initialized:
                    self._status_field.model.set_value(f"Error reading data from PLC: {e}")
                    status_update_time = time.time() + 1
                time.sleep(1)

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

    def _on_plc_port_changed(self, value):
        self._websockets_connector.port = value.get_value_as_string()
        self._communication_initialized = False

    def _on_refresh_rate_changed(self, value):
        self._refresh_rate = value.get_value_as_int()

    def _toggle_communication_enable(self, state):
        self._enable_communication = state.get_value_as_bool()
        if not self._enable_communication:
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

