import unittest
from parse_name import _parse_name

class TestParseName_1_SingleVar(unittest.TestCase):

    def test_start_empty(self):
        starting_dict = {}
        value = 30
        input = "gBool"
        correct_output = {input: value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_start_nonempty(self):
        starting_dict = {"gInt" : 7}
        value = 30
        input = "gBool"
        correct_output = {"gInt" : 7, "gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_override(self):
        starting_dict = {"gBool" : 1}
        value = 30
        input = "gBool"
        correct_output = {"gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_overwriteList(self):
        starting_dict = {"gBool" : ['a', 'b', 'c']}
        value = 30
        input = "gBool"
        correct_output = {"gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_overwriteDict(self):
        starting_dict = {"gBool" : {'a': 1}}
        value = 30
        input = "gBool"
        correct_output = {"gBool": value}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)


class TestParseName_2_SingleArr(unittest.TestCase):
    
    def test_start_empty(self):
        starting_dict = {}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_start_nonempty(self):
        starting_dict = {"gInt" : 7}
        value = 30
        input = "gBool[2]"
        correct_output = {"gInt" : 7, "gBool": [None, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)
    
    def test_exists_empty_list(self):
        starting_dict = {"gBool" : []}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)
    
    def test_exists_short_list(self):
        starting_dict = {"gBool" : [1]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [1, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_exists_right_size_list(self):
        starting_dict = {"gBool" : [1, 2, 3]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [1, 2, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)
    
    def test_exists_long_list(self):
        starting_dict = {"gBool" : [1, 2, 3, 4, 5]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [1, 2, value, 4, 5]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)
    
    def test_exists_num(self):
        starting_dict = {"gBool" : 40}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)
    
    def test_exists_dict(self):
        starting_dict = {"gBool" : {'a' : 1}}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}
        actual_output = _parse_name(name_dict=starting_dict, name=input, value=value)
        self.assertEqual(actual_output, correct_output)


class TestParseName_3_Multipart(unittest.TestCase):

    def test_top_struct(self) -> None:

        value = 30
        input = "myStruct.subStruct.var"
        correct_output = {  
            "myStruct": {
                "subStruct": {
                    "var": value
                    }
                }
            }
        actual_output = _parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_task_struct(self) -> None:

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

    def test_task_array_member(self) -> None:

        value = 30
        array_size = 30
        input = "Program:array[29]"
        correct_output = {"Program": {"array" : [None] * array_size}}
        correct_output["Program"]["array"][array_size - 1] = value
        
        actual_output = _parse_name(name_dict={}, name=input, value=value)
        msg = "\n" + str(correct_output) + "\n" + str(actual_output)
        self.assertEqual(actual_output, correct_output, msg=msg)

    def test_array_index_member_first(self):
        value = 30
        actual_output = _parse_name({}, "myArray[0].myVar", value)
        expected =  {
                        "myArray": 
                        [
                            {"myVar": value}
                        ]
                    }
                    
        self.assertEqual(actual_output, expected)

    def test_array_index_member_nth(self):
        value = 30
        actual_output = _parse_name({}, "myArray[2].myVar", value)
        expected =  {
                        "myArray": 
                        [   None,
                            None,
                            {"myVar": value}
                        ]
                    }
        self.assertEqual(actual_output, expected)

    def test_array_index_member_nth_existing_replace(self):
        starting_dict = {'myVar' : 1, "myArray": [1, 2, 3, 4]}
        value = 30
        actual_output = _parse_name(starting_dict, "myArray[2].myVar", value)
        expected =  {
                        "myVar" : 1,
                        "myArray": 
                        [   1,
                            2,
                            {"myVar": value},
                            4
                        ]
                    }
        self.assertEqual(actual_output, expected)

    def test_structure_array_index_member(self):
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


class TestParseName_4_Misc(unittest.TestCase):

    def test_deep_mix_of_nesting(self):
        value = 30
        actual_output = _parse_name({}, "Task:myStruct.myArray[1].myStruct.arr[3].myVar", value)
        expected =  {   "Task": {
                            "myStruct": 
                            {
                                "myArray": 
                                [
                                    None,
                                    {
                                        "myStruct": {
                                            "arr": [None, None, None, {"myVar" : value}]
                                            }
                                    }
                                ]
                            }
                        }
                    }
        self.assertEqual(actual_output, expected)









if __name__ == '__main__':
    unittest.main()