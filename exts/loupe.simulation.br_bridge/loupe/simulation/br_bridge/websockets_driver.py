'''
  File: **websockets_driver.py**
  Copyright (c) 2024 Loupe
  https://loupe.team
  
  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.
  
'''

import asyncio
import websockets
import json
import re

import websockets.client

class PLCDataParsingException(Exception):
    pass

class WebsocketsDriver():
    """
    A class that represents an websockets driver. It contains a list of variables to read from the target device and provides methods to read and write data.

    Attributes:
        ip (string): ip address of the PLC
        port (int): port of the PLC
        connection (WebSocketClientProtocol):
        _read_names (list): A list of plc var names for reading data.

    """

    def __init__(self, ip=None, port=None):       
        """
        Initializes an instance of the WebsocketsDriver class.

        """
        self.ip = ip
        self.port = port
        self._connection = None

        self._read_names = list()

    def add_read(self, plc_var : str):
        """
        Adds a variable to the cyclic read list.

        Args:
            plc_var (str): The plc_var of the data to be read. "Program:my_struct.my_array[0].my_var"

        """
        if plc_var not in self._read_names:
            self._read_names.append(plc_var)

    def clear_read_list(self):
        """Clear the current list of variables to read from the PLC."""
        self._read_names = []

    async def write_data(self, data : dict ):
        """
        Writes data to the PLC.

        Args:
            data (dict): A dictionary containing the data to be written to the PLC
            e.g.
            data = {'MAIN:b_Execute': False, 'MAIN:str_TestString': 'Goodbye World', 'MAIN:r32_TestReal': 54.321}

        """
        payload = {
            "type": "write",
            "data": data
        }
        payload_json = json.dumps(payload)
        await self._connection.send(payload_json)

    async def read_data(self):
        """
        Reads all variables from the cyclic read list.

        Returns:
            dict: A dictionary containing the parsed data from the PLC

        """
        plc_var_dict = {}

        if not self._read_names:
            return plc_var_dict

        # Send request for data
        payload_obj = {
            "type": "read",
            "data": self._read_names
        }
        payload_json = json.dumps(payload_obj)

        results = await asyncio.gather(
            self._connection.send(payload_json),
            self._connection.recv()
        )
        response_json = results[1] # get results second gather call

        # Wait for response
        response = json.loads(response_json)

        if "data" not in response:
            raise PLCDataParsingException("No data in response")
        elif "type" not in response:
            raise PLCDataParsingException("No type in response")
        else:
            plc_var_dict = self._parse_plc_response(response)
            
        return plc_var_dict
    
    def _parse_plc_response(self, response):
        """
        Parses the dictionary of variables sent from the PLC.
        This function assumes response is a dictionary with a "type" and "data" key
        
        Args:
            response (dict): A dictionary containing the data to be parsed
        """
        plc_var_dict = {}
        if response["type"] == "readresponse":
            try:
                for plc_var_dict in response["data"]:
                    for plc_var, plc_var_value in plc_var_dict.items():
                        plc_var_dict = self._parse_flat_plc_var_to_dictionary(plc_var_dict,
                                                                              plc_var,
                                                                              plc_var_value)
            except Exception as e:
                raise PLCDataParsingException(str(e)) from e
        elif response["type"] == "writeresponse":
            pass
        return plc_var_dict
    
    def _parse_name(self, name_dict, name, value):

        name_parts = re.split('[:.]', name)

        if len(name_parts) > 1:
            # Multiple parts to passed-in name (e.g. Program:myStruct[3].myVar has 3 parts)
            # From here we want to use recursion to assign a dictionary value (i.e. sub dictionary) to the first part.

            first_part_is_array = '[' in name_parts[0]

            ## Ensure corresponding subdictionary exists
            if first_part_is_array:
                array_name, index = name_parts[0].split("[")
                index = int(index[:-1])
                
                if array_name not in name_dict or not isinstance(name_dict[array_name], list):
                    name_dict[array_name] = []
                
                # Extend if necessary
                if index >= len(name_dict[array_name]):
                    name_dict[array_name].extend([None] * (index - len(name_dict[array_name]) + 1))

                # Ensure array index location has dict-typed value
                if not isinstance(name_dict[array_name][index], dict):
                    name_dict[array_name][index] = {}
                    
                existing_sub_dict = name_dict[array_name][index]        
            else:
                member_name = name_parts[0]
                
                ## Ensure corresponding subdictionary exists
                if member_name not in name_dict or not isinstance(name_dict[member_name], dict):
                    name_dict[member_name] = {}
                
                existing_sub_dict = name_dict[member_name]
            
            # Get subdictionary from using remaining part of path
            sub_name = '.'.join(name_parts[1:])
            sub_dict = self._parse_name(existing_sub_dict , sub_name, value)
            
            if first_part_is_array:
                name_dict[array_name][index] = sub_dict
            else:
                name_dict[member_name] = sub_dict
        else:
            # only one part to passed in name

            if '[' in name_parts[0]:
                array_name, index = name_parts[0].split("[")
                index = int(index[:-1])
                
                if array_name not in name_dict or not isinstance(name_dict[array_name], list):
                    name_dict[array_name] = []
                
                # Extend if necessary
                if index >= len(name_dict[array_name]):
                    name_dict[array_name].extend([None] * (index - len(name_dict[array_name]) + 1))
                
                name_dict[array_name][index] = value
            else:
                # Write value (regardless of whether it exists or not)
                name_dict[name_parts[0]] = value
                
        return name_dict
    
    async def connect(self):
        """
        Connects to the target device.

        """
        self._connection = await websockets.client.connect("ws://" + self.ip + ":" + str(self.port), ping_interval=None)

    async def disconnect(self):
        """
        Disconnects from the target device.

        """
        await self._connection.close()

    def is_connected(self):
        """
        Returns the connection state.

        Returns:
            bool: True if the connection is open, False otherwise.

        """
        return self._connection.open