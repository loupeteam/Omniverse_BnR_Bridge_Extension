import re

# Ensure that dictionary has a key of list_name, that it's value is a list,
#   and that the list is long enough to include the index
def _ensure_list_with_index_in_dict(list_name, _dict, _index):
    # Create list if not in dict
    if list_name not in _dict or not isinstance(_dict[list_name], list):
        _dict[list_name] = []

    # Extend list if not long enough
    if _index >= len(_dict[list_name]):
        _dict[list_name].extend([None] * (_index - len(_dict[list_name]) + 1))

# Parse name and insert value into correct location in name_dict, creating a location
#   in the name_dict if necessary. Returns modified name_dict.
def _parse_name(name_dict, name, value):

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
            _ensure_list_with_index_in_dict(array_name, name_dict, index)

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
        sub_dict = _parse_name(existing_sub_dict, sub_name, value)
        
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
            _ensure_list_with_index_in_dict(array_name, name_dict, index)

            name_dict[array_name][index] = value
        else:
            # Write value (regardless of whether it exists or not)
            name_dict[name_parts[0]] = value
            
    return name_dict

