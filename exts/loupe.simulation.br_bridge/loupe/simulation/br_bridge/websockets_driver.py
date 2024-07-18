'''
  File: **websockets_driver.py**
  Copyright (c) 2024 Loupe
  https://loupe.team
  
  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.
  
'''

import websockets
import json
import re

class PLCDataParsingException(Exception):
    pass

class WebsocketsDriver():
    """
    A class that represents an websockets driver. It contains a list of variables to read from the target device and provides methods to read and write data.

    Attributes:
        ip (string): ip address of the PLC
        port (int): port of the PLC
        connection (WebSocketClientProtocol):
        _read_names (list): A list of names for reading data.
        _read_struct_def (dict): A dictionary that maps names to structure definitions.

    """

    def __init__(self, ip=None, port=None):             
        """
        Initializes an instance of the WebsocketsDriver class.

        """
        self.ip = ip
        self.port = port
        self._connection = None

        self._read_names = list()
        self._read_struct_def = dict()

    def add_read(self, name : str, structure_def = None):
        """
        Adds a variable to the list of data to read.

        Args:
            name (str): The name of the data to be read. "my_struct.my_array[0].my_var"
            structure_def (optional): The structure definition of the data.

        """
        if name not in self._read_names:
            self._read_names.append(name)

        if structure_def is not None:
            if name not in self._read_struct_def:
                self._read_struct_def[name] = structure_def

    def clear_read_list(self):
        self._read_names = []

    async def write_data(self, data : dict ):
        """
        Writes data to the target device.

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
        parsed_data = {}
        
        if not self._read_names:
            return parsed_data

        # Send request for data
        payload_obj = {
            "type": "read",
            "data": self._read_names
        }
        payload_json = json.dumps(payload_obj)
        await self._connection.send(payload_json)
        # Wait for response
        response_json = await self._connection.recv()
        response = json.loads(response_json)

        if "data" not in response:
            raise PLCDataParsingException("No data in response")
        elif "type" not in response:
            raise PLCDataParsingException("No type in response")
        else:
            parsed_data = self._parse_plc_response(response)
            
        return parsed_data
    
    # This function assumes response is a dictionary with a "type" and "data" key
    def _parse_plc_response(self, response):
        parsed_data = {}
        if response["type"] == "readresponse":
            try:
                for plc_var_dict in response["data"]:
                    for key, value in plc_var_dict.items():
                        parsed_data = self._parse_name(parsed_data, key, value)
            except Exception as e:
                raise PLCDataParsingException(str(e))
        elif response["type"] == "writeresponse":
            print('succesfully wrote data')
        return parsed_data

    
    # Ensure that dictionary has a key of list_name, that it's value is a list,
    #   and that the list is long enough to include the index
    def _ensure_list_with_index_in_dict(self, list_name, _dict, _index):
        # Create list if not in dict
        if list_name not in _dict or not isinstance(_dict[list_name], list):
            _dict[list_name] = []

        # Extend list if not long enough
        if _index >= len(_dict[list_name]):
            _dict[list_name].extend([None] * (_index - len(_dict[list_name]) + 1))

    # Parse name and insert value into correct location in name_dict, creating a location
    #   in the name_dict if necessary. Returns modified name_dict.
    def _parse_name(self, name_dict, name, value):

        # Split name into parts
        # Example: "Program:myStruct[3].myVar" -> ["Program", "myStruct[3]", "myVar"]
        name_parts = re.split('[:.]', name)

        if len(name_parts) > 1:
            # Multiple parts in passed-in name (e.g. Program:myStruct[3].myVar has 3 parts)
            # From here we want to use recursion to assign a dictionary value (i.e. sub dictionary) to the first part.

            first_part_is_array = '[' in name_parts[0]

            ## Get pre-existing subdictionary (or create if necessary)
            if first_part_is_array:
                array_name, index = name_parts[0].split("[")
                index = int(index[:-1])
                
                # Ensure array is in dictionary and is long enough
                self._ensure_list_with_index_in_dict(array_name, name_dict, index)

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
            sub_dict = self._parse_name(existing_sub_dict, sub_name, value)
            
            # Assign result of recursive call (subdictionary) to first part
            if first_part_is_array:
                name_dict[array_name][index] = sub_dict
            else:
                name_dict[member_name] = sub_dict
        else:
            # Only one part in passed-in name
            # Proceed to assign value

            if '[' in name_parts[0]:
                array_name, index = name_parts[0].split("[")
                index = int(index[:-1])
                
                # Ensure array is in dictionary and is long enough
                self._ensure_list_with_index_in_dict(array_name, name_dict, index)

                name_dict[array_name][index] = value
            else:
                # Write value (regardless of whether it exists or not)
                name_dict[name_parts[0]] = value
                
        return name_dict
    
    async def connect(self):
        """
        Connects to the target device.

        """
        self._connection = await websockets.connect("ws://" + self.ip + ":" + str(self.port))

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


