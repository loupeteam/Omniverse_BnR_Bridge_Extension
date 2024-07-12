'''
  File: **websockets_driver.py**
  Copyright (c) 2024 Loupe
  https://loupe.team
  
  This file is part of Omniverse_BuR_Bridge_Extension, licensed under the MIT License.
  
'''


class WebsocketsDriver():
    """
    A class that represents an websockets driver. It contains a list of variables to read from the target device and provides methods to read and write data.

    Attributes:
        ip (string): ip address of the PLC
        port (int): port of the PLC
        _read_names (list): A list of names for reading data.
        _read_struct_def (dict): A dictionary that maps names to structure definitions.

    """

    def __init__(self, ip, port):             
        """
        Initializes an instance of the WebsocketsDriver class.

        Args:
            ip (string): ip address of the PLC
            port (int): port of the PLC

        """
        self.ip = ip
        self.port = port
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

    def write_data(self, data : dict ):
        """
        Writes data to the target device.

        Args:
            data (dict): A dictionary containing the data to be written to the PLC
            e.g.
            data = {'MAIN:b_Execute': False, 'MAIN:str_TestString': 'Goodbye World', 'MAIN:r32_TestReal': 54.321}

        """
        self._connection.write_list_by_name(data)

    def read_data(self):
        """
        Reads all variables from the cyclic read list.

        Returns:
            dict: A dictionary containing the parsed data.

        """
        if self._read_names.__len__() > 0:
            data = self._connection.read_list_by_name(self._read_names, structure_defs=self._read_struct_def)
            parsed_data = dict()
            for name in data.keys():
                parsed_data = self._parse_name(parsed_data, name, data[name])
        else:
            parsed_data = dict()        
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
        # TODO update to work with "Task:var" convention (not "Task.var")
        name_parts = name.split(".")
        if len(name_parts) > 1:
            if name_parts[0] not in name_dict:
                name_dict[name_parts[0]] = dict()
            if "[" in name_parts[1]:
                array_name, index = name_parts[1].split("[")
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
                array_name, index = name_parts[0].split("[")
                index = int(index[:-1])
                if index >= len(name_dict):
                    name_dict.extend([None] * (index - len(name_dict) + 1))
                name_dict[index] = value
                return name_dict[index]
            else:
                name_dict[name_parts[0]] = value
        return name_dict
    
    def connect(self, ip, port):
        """
        Connects to the target device.

        Args:
            ip (string): ip address of the PLC
            port (int): port of the PLC

        """
        
        self._connection.open()

    def disconnect(self):
        """
        Disconnects from the target device.

        """
        self._connection.close()

    def is_connected(self):
        """
        Returns the connection state.

        Returns:
            bool: True if the connection is open, False otherwise.

        """
        try:
            # TODO
            return True
        except Exception as e:
            return False


