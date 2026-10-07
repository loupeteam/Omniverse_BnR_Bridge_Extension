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
from ..common.SystemUI import SystemUI, LABEL_WIDTH


class UIBuilder(SystemUI):
    """
    UI Builder for the B&R Bridge runtime component
    """
    def build_component_ui_runtime(self):
        with ui.HStack(spacing=5, height=0):
            ui.Label("Enable Client", width=LABEL_WIDTH)
            self._enable_communication_checkbox = ui.CheckBox(
                ui.SimpleBoolModel(self.active_runtime.enable_communication)
            )
            self._enable_communication_checkbox.model.add_value_changed_fn(
                self._toggle_communication_enable
            )

        with ui.HStack(spacing=5, height=0):
            ui.Label("Refresh Rate (ms)", width=LABEL_WIDTH)
            self._refresh_rate_field = ui.IntField(
                ui.SimpleIntModel(self.active_runtime.refresh_rate)
            )
            self._refresh_rate_field.model.set_min(10)
            self._refresh_rate_field.model.set_max(10000)
            self._refresh_rate_field.model.add_end_edit_fn(
                self._on_refresh_rate_changed
            )

        with ui.HStack(spacing=5, height=0):
            ui.Label("PLC IP Address", width=LABEL_WIDTH)
            self._plc_host_field = ui.StringField(
                ui.SimpleStringModel(self.active_runtime.host)
            )
            self._plc_host_field.model.add_end_edit_fn(self._on_plc_host_changed)

        with ui.HStack(spacing=5, height=0):
            ui.Label("PLC Port", width=LABEL_WIDTH)
            self._plc_port_field = ui.IntField(
                ui.SimpleIntModel(self.active_runtime.port)
            )
            self._plc_port_field.model.set_min(0)
            self._plc_port_field.model.set_max(65535)
            self._plc_port_field.model.add_end_edit_fn(self._on_plc_port_changed)

        with ui.CollapsableFrame("Cyclic Read Variables", collapsed=True):
            with ui.VStack(spacing=5, height=200):
                ui.Label("1 Variable per line", width=LABEL_WIDTH, height=0)
                self._variables_field = ui.StringField(
                    ui.SimpleStringModel(
                        "\n".join(
                            self.active_runtime.read_variables
                        )
                    ),
                    multiline=True,
                )
                self._variables_field.model.add_end_edit_fn(
                    self._on_variables_changed
                )

    def _on_plc_host_changed(self, value):
        self.active_runtime.host = value.get_value_as_string()

    def _on_plc_port_changed(self, value):
        self.active_runtime.port = value.get_value_as_int()

    def _on_refresh_rate_changed(self, value):
        self.active_runtime.refresh_period_ms = value.get_value_as_int()

    def _toggle_communication_enable(self, state):
        self.active_runtime.enable_communication = state.get_value_as_bool()

    def _on_variables_changed(self, value):
        variables = value.get_value_as_string().split("\n")
        self.active_runtime.set_read_variables(variables)
