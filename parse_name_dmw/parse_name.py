import re

def _parse_name(name_dict, name, value):

    name_parts = re.split('[:.]', name)

    if len(name_parts) > 1:
    
        # Get existing sub dictionary representing name_parts[0]
        
        ## Ensure corresponding subdictionary exists
        first_part_is_array = '[' in name_parts[0]
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
        
        
        sub_name = '.'.join(name_parts[1:])
        sub_dict = _parse_name(existing_sub_dict , sub_name, value)
        
        if first_part_is_array:
            name_dict[array_name][index] = sub_dict
        else:
            name_dict[member_name] = sub_dict
    else:
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
            name_dict[name_parts[0]] = value
            
    return name_dict

