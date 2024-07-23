'''
  File: **websockets_driver.py**
  Copyright (c) 2024 Loupe
  https://loupe.team
  
  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.
  
'''

import asyncio
import json
import time
import re

import websockets.client
from websockets.exceptions import ConnectionClosedError

class PLCDataParsingException(Exception):
    pass

class WebsocketsConnectionException(Exception):
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
        response_json = results[1] # get the return from results' second function

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
                for var_dict in response["data"]:
                    for plc_var, plc_var_value in var_dict.items():
                        plc_var_dict = self._parse_flat_plc_var_to_dict(plc_var_dict,
                                                                        plc_var,
                                                                        plc_var_value)
            except Exception as e:
                raise PLCDataParsingException(str(e)) from e
        elif response["type"] == "writeresponse":
            print('succesfully wrote data')
        return plc_var_dict


    def _ensure_list_with_index_in_dict(self, list_name, _dict, _index):
        """
        Ensure that dictionary has a key of list_name, that it's value is a list,
        and that the list is long enough to include the index
        """

        # Create list if not in dict
        if list_name not in _dict or not isinstance(_dict[list_name], list):
            _dict[list_name] = []

        # Extend list if not long enough
        if _index >= len(_dict[list_name]):
            _dict[list_name].extend([None] * (_index - len(_dict[list_name]) + 1))
    
    def _parse_flat_plc_var_to_dict(self, plc_var_dict, plc_var, value):
        """
        Convert a flat, string representation of a PLC var into a dictionary.

        This function uses recursion to build up the complete dictionary of PLC variables, and values.
        
        This is performed every read, rather than being cached, to not assume PLC variable values
        to be at their previous value if they are not being actively read. Caching can be
        performed in the usage of this library if necessary.

        Args:
            plc_var_dict (dict): The dictionary to write the value into
            plc_var (str): The variable name in flattened string form ("Program:myStruct.myVar")
            value (any): The value to write to the dictionary entry
        """

        name_parts = re.split('[:.]', plc_var)

        if len(name_parts) > 1:
            # Multiple parts in passed-in plc_var (e.g. Program:myStruct[3].myVar has 3 parts)
            # From here we want to use recursion to assign a dictionary value (i.e. sub dictionary) to the first part.

            first_part_is_array = '[' in name_parts[0]

            ## Get pre-existing subdictionary (or create if necessary)
            if first_part_is_array:
                array_name, array_index = name_parts[0].split("[")
                array_index = int(array_index[:-1])
                
                # Ensure array is in dictionary and is long enough
                self._ensure_list_with_index_in_dict(array_name, plc_var_dict, array_index)

                # Ensure array index location has dict-typed value
                if not isinstance(plc_var_dict[array_name][array_index], dict):
                    plc_var_dict[array_name][array_index] = {}
                    
                existing_sub_dict = plc_var_dict[array_name][array_index]
            else:
                member_plc_var = name_parts[0]
                
                ## Ensure corresponding subdictionary exists
                if member_plc_var not in plc_var_dict or not isinstance(plc_var_dict[member_plc_var], dict):
                    plc_var_dict[member_plc_var] = {}
                
                existing_sub_dict = plc_var_dict[member_plc_var]
            
            # Get subdictionary from using remaining part of path
            sub_plc_var = '.'.join(name_parts[1:])
            sub_dict = self._parse_flat_plc_var_to_dict(existing_sub_dict,
                                                              sub_plc_var,
                                                              value)
            
            # Assign result of recursive call (subdictionary) to first part
            if first_part_is_array:
                plc_var_dict[array_name][array_index] = sub_dict
            else:
                plc_var_dict[member_plc_var] = sub_dict
        else:
            # Only one part in passed-in plc_var
            # Proceed to assign value

            if '[' in name_parts[0]:
                array_name, array_index = name_parts[0].split("[")
                array_index = int(array_index[:-1])
                
                # Ensure array is in dictionary and is long enough
                self._ensure_list_with_index_in_dict(array_name, plc_var_dict, array_index)

                plc_var_dict[array_name][array_index] = value
            else:
                # Write value (regardless of whether it exists or not)
                plc_var_dict[name_parts[0]] = value
                
        return plc_var_dict
    
    async def connect(self):
        """
        Connects to the target device.

        Returns True if connection was succesful, False otherwise.

        """
        try:
            self._connection = await websockets.client.connect("ws://" + self.ip + ":" + str(self.port),
                                                               open_timeout=3,
                                                               ping_interval=None,  # OMJSON does not use ping/pong
                                                               close_timeout=1) # Could potentially be shorter
        except ConnectionClosedError as e:
            raise WebsocketsConnectionException("Connection Closed Error: " + str(e)) from e
        except asyncio.TimeoutError as e:
            raise WebsocketsConnectionException("Connecting..." + str(e)) from e
        except ConnectionRefusedError as e:
            raise WebsocketsConnectionException("Connection Refused Error, check IP and Port: " + str(e)) from e

        if self._connection:
            return self._connection.open
        else:
            return False
        
    async def disconnect(self):
        """
        Disconnects from the target device.

        """
        if self._connection and self._connection.open:
                # OMJSON doesn't support the connection close opCode. This forces a close.
                await(self._connection.send(''))
                time.sleep(.25) # Giving time for message to be processed by PLC
                await self._connection.close()

    def is_connected(self):
        """
        Returns the connection state.

        Returns:
            bool: True if the connection is open, False otherwise.

        """
        if self._connection:
            return self._connection.open
        else:
            return False