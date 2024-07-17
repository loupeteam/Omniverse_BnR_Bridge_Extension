
# run like a normal python script and observe results

import unittest
from parse_name import _parse_name

class TestParseName(unittest.TestCase):

    def test_variable_name_parsing_global(self):
        starting_dict = {}
        value = 30
        input = "gBool"
        correct_output = {input: value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_global_overwrite(self):
        starting_dict = {"gBool" : 1}
        value = 30
        input = "gBool"
        correct_output = {"gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_global_overwriteList(self):
        starting_dict = {"gBool" : ['a', 'b', 'c']}
        value = 30
        input = "gBool"
        correct_output = {"gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_paring_global_addition(self):
        starting_dict = {"gInt" : 7}
        value = 30
        input = "gBool"
        correct_output = {"gInt" : 7, "gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    
    def test_variable_name_parsing_globalArr(self):

        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}
        actual_output = _parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    
    def test_variable_name_parsing_globalArr_addition0(self):
        starting_dict = {"gInt" : 7, "gBool" : []}
        value = 30
        input = "gBool[2]"
        correct_output = {"gInt" : 7, "gBool": [None, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    
    def test_variable_name_parsing_globalArr_addition1(self):
        starting_dict = {"gInt" : 7, "gBool" : [1]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gInt" : 7, "gBool": [1, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    
    def test_variable_name_parsing_globalArr_addition123(self):
        starting_dict = {"gInt" : 7, "gBool" : [1, 2, 3]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gInt" : 7, "gBool": [1, 2, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    
    def test_variable_name_parsing_globalArr_addition(self):
        starting_dict = {"gInt" : 7, "gBool" : [1, 2]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gInt" : 7, "gBool": [1, 2, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)


    def test_variable_name_parsing_simple_struct(self) -> None:

        value = 30
        input = "Program:struct.var"
        correct_output = {
            "Program": {
                "struct": {
                    "var": value}
                }
            }
        actual_output = _parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_simple_array(self) -> None:

        value = 30
        array_size = 30
        input = "Program:array[29]"
        correct_output = {"Program": {"array" : [None] * array_size}}
        correct_output["Program"]["array"][array_size - 1] = value
        
        actual_output = _parse_name(name_dict={}, name=input, value=value)
        msg = "\n" + str(correct_output) + "\n" + str(actual_output)
        self.assertEqual(actual_output, correct_output, msg=msg)



    def test_variable_name_parsing_array_member(self):
        value = 30
        actual_output = _parse_name({}, "myArray[0].myVar", value)
        expected =  {
                        "myArray": 
                        [
                            {"myVar": value}
                        ]
                    }
                    
        self.assertEqual(actual_output, expected)

    def test_variable_name_parsing_array_member_deeper(self):
        value = 30
        actual_output = _parse_name({}, "myStruct.myArray[0].myVar", value)
        expected =  {
                        "myStruct": 
                        {
                            "myArray": 
                            [
                                {"myVar": value}
                            ]
                        }
                    }
        self.assertEqual(actual_output, expected)










if __name__ == '__main__':
    unittest.main()