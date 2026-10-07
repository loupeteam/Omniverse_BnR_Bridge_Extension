# This software contains source code provided by NVIDIA Corporation.
# Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#


EXTENSION_TITLE = "B&R Bridge"
EXTENSION_NAME = "loupe.simulation.br_bridge"
EXTENSION_DESCRIPTION = "Bridge to B&R PLCs"

ATTR_BR_BRIDGE_HOST = "br_bridge:Host"
ATTR_BR_BRIDGE_PORT = "br_bridge:Port"
ATTR_BR_BRIDGE_ENABLE = "br_bridge:Enable"
ATTR_BR_BRIDGE_REFRESH = "br_bridge:RefreshRate"
ATTR_BR_BRIDGE_READ_VARS = "br_bridge:Variables"

"""
    These are the default properties for the B&R Bridge when creating a new component
"""
default_br_properties = {
    ATTR_BR_BRIDGE_ENABLE: False,
    ATTR_BR_BRIDGE_REFRESH: 20,
    ATTR_BR_BRIDGE_HOST: "127.0.0.1",
    ATTR_BR_BRIDGE_PORT: 8000,
    ATTR_BR_BRIDGE_READ_VARS: "",  # Ideally this should be a list of variables, but they aren't support on the gui
}
