'''
  File: **websockets_driver.py**
  Copyright (c) 2024 Loupe
  https://loupe.team
  
  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.
  
'''

import websockets
import json
import re

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
        await self.connection.send(payload_json)

    async def read_data(self):
        """
        Reads all variables from the cyclic read list.

        Returns:
            dict: A dictionary containing the parsed data from the PLC

        """
        if self._read_names:
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
            # TODO what if its a write response? Does this function process both?
            response_type = response["type"]
            if response_type == "readresponse":
                #print(response["data"])
                parsed_data = {}
                # TODO bypassing parsing for now
                for name in response["data"]:
                    parsed_data = self._parse_name(parsed_data, name, response[name])
            elif response_type == "writeresponse":
                print('wrote data')
                parsed_data = {}
        else:
            parsed_data = {}
        return parsed_data
        
    def _parse_name(self, name_dict, name, value):
        """
        Convert a variable from a flat name to a dictionary based structure.

        "my_struct.my_array[0].my_var: value" -> {"my_struct": {"my_array": [{"my_var": value}]}}

        Args:
            name_dict (dict): The dictionary to store the parsed data.
            name (str): The name of the data item.
            value: The value of the data item.

        Returns:
            dict: The updated name_dict.

        """
        try:
            # split PLC var name by : or , characters
            name_parts = re.split('[:.]', name)
            # If there's more than one level of hierarchy (var1.var2.var3 vs (var1)
            if len(name_parts) > 1:
                # if var1 isn't already in the dictionary. create a dictionary for it
                if name_parts[0] not in name_dict:
                    name_dict[name_parts[0]] = dict()
                # is the next level an array (var2)
                if "[" in name_parts[1]:
                    # name_parts[1] is var2[0] (with there maybe being something after it)
                    array_name, index = name_parts[1].split("[")
                    # strip off the ], convert index to an int from a str
                    index = int(index[:-1])
                    if array_name not in name_dict[name_parts[0]]:
                        name_dict[name_parts[0]][array_name] = []
                    if index >= len(name_dict[name_parts[0]][array_name]):
                        name_dict[name_parts[0]][array_name].extend([None] * (index - len(name_dict[name_parts[0]][array_name]) + 1))
                    name_dict[name_parts[0]][array_name][index] = self._parse_name(name_dict[name_parts[0]][array_name], "[" + str(index) + "]" + ".".join(name_parts[2:]), value)
                else:
                    name_dict[name_parts[0]] = self._parse_name(name_dict[name_parts[0]], ".".join(name_parts[1:]), value)
            else:
                
                if "[" in name_parts[0]:
                    # last element is an array - so we started with Task:myArray[0] => myArray[0]
                    # or we're at the end of recursion, myArray[0]
                    array_name, index = name_parts[0].split("[")  # array_name is myArray, index is "0]"
                    index = int(index[:-1]) # remove "]"
                    if array_name not in name_dict or not isinstance(name_dict[array_name], list):
                        name_dict[array_name] = []
                    
                    if index >= len(name_dict[array_name]):
                        name_dict[array_name].extend([None] * (index - len(name_dict[array_name]) + 1))
                    name_dict[array_name][index] = value
                else:
                    # myNotArrayAlsoNotAStruct
                    name_dict[name_parts[0]] = value
        except Exception as e:
            print('generic exception' + str(e))
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


