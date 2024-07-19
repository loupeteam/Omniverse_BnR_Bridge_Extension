name_dict = {}
value = "30"
array_size = 30
passes = 0
fails = 0

import omni.kit.test
from loupe.simulation.br_bridge.websockets_driver import WebsocketsDriver

class TestParseName_SingleVar(omni.kit.test.AsyncTestCase):

    # Run before every test
    async def setUp(self):
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
        self.test_output_string = "correct: {correct}\nactual: {actual}\n\n"
        self.test_different_data_types = ["30", 30, -18935, 30.151535, True, False]
    
    # Run after every test
    async def tearDown(self):
        pass

    def test_start_empty(self):
        """Empty dictionary is populated with single var."""
        starting_dict = {}
        value = 30
        input = "gVar"
        correct_output = {input: value}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_start_nonempty(self):
        """Non-empty dictionary is populated with second var."""
        starting_dict = {"gInt" : 7}
        value = 30
        input = "gOtherInt"

        for value in self.test_different_data_types:
            correct_output = {"gInt" : 7, input: value}

            actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)
            
            self.assertEqual(actual_output, 
                            correct_output, 
                            msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_overwrite(self):
        """Non-empty dictionary has a single var overwritten."""
        starting_dict = {"gVar" : 1}
        input = "gVar"

        for value in self.test_different_data_types:
            correct_output = {"gVar": value}
            actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

            self.assertEqual(actual_output, 
                            correct_output, 
                            msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_overwrite_string_list(self):
        """Non-empty dictionary has a string list ovewritten with a different var type."""
        starting_dict = {"gArray" : ['a', 'b', 'c']}
        value = 30
        input = "gArray"
        correct_output = {"gArray": value}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_overwrite_struct(self):
        """Overwrite a struct with a different var type."""
        starting_dict = {"gStruct" : {'a': 1}}
        value = 30
        input = "gStruct"
        correct_output = {"gStruct": value}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


class TestParseName_SingleArray(omni.kit.test.AsyncTestCase):

    # Run before every test
    async def setUp(self):
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
        self.test_output_string = "correct: {correct}\nactual: {actual}\n\n"
        self.test_different_data_types = ["30", 30, -18935, 30.151535, True, False]
    
    # Run after every test
    async def tearDown(self):
        pass
    
    def test_start_empty(self):
        """Add a list to an existing dictionary"""
        starting_dict = {}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_start_nonempty(self):
        """Read a single element of a list, from a dict already containing data."""
        starting_dict = {"gInt" : 7}
        value = True
        input = "gBool[2]"
        correct_output = {"gInt" : 7, "gBool": [None, None, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    
    def test_exists_empty_list(self):
        """Read a single element of a list, whose reprsentation is currently an empty list."""
        starting_dict = {"gBool" : []}
        value = False
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    
    def test_exists_short_list(self):
        """Read a single element of a list, whose reprsentation is currently a list that's too short."""
        starting_dict = {"gBool" : [False]}
        value = True
        input = "gBool[2]"
        correct_output = {"gBool": [False, None, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_exists_right_size_list(self):
        """Read a single element of a list, whose representation is a list where requested index is the last element."""
        starting_dict = {"gInt" : [1, 2, 3]}
        value = 30
        input = "gInt[2]"
        correct_output = {"gInt": [1, 2, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    
    def test_exists_long_list(self):
        starting_dict = {"gBool" : [1, 2, 3, 4, 5]}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [1, 2, value, 4, 5]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    
    def test_exists_num(self):
        starting_dict = {"gBool" : 40}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    
    def test_exists_dict(self):
        starting_dict = {"gBool" : {'a' : 1}}
        value = 30
        input = "gBool[2]"
        correct_output = {"gBool": [None, None, value]}

        actual_output = self.driver._parse_name(name_dict=starting_dict, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))



class TestParseName_Multipart(omni.kit.test.AsyncTestCase):

        # Run before every test
    async def setUp(self):
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
        self.test_output_string = "correct: {correct}\nactual: {actual}\n\n"
        self.test_different_data_types = ["30", 30, -18935, 30.151535, True, False]
    
    # Run after every test
    async def tearDown(self):
        pass

    def test_top_struct(self):

        value = 30
        input = "myStruct.subStruct.var"
        correct_output = {  
            "myStruct": {
                "subStruct": {
                    "var": value
                    }
                }
            }
        
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_task_level_struct(self):
        """Test writing to a struct inside of a program"""
        value = 30
        input = "Program:struct.var"
        correct_output = {
            "Program": {
                "struct": {
                    "var": value}
                }
            }
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_task_array_member(self):

        value = 30
        array_size = 30
        input = "Program:array[29]"
        correct_output = {"Program": {"array" : [None] * array_size}}
        correct_output["Program"]["array"][array_size - 1] = value
        
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


    def test_array_index_member_first(self):
        value = 30
        actual_output = self.driver._parse_name({}, "myArray[0].myVar", value)
        correct_output =  {
                        "myArray": 
                        [
                            {"myVar": value}
                        ]
                    }
                    
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_array_index_member_nth(self):
        value = 30
        actual_output = self.driver._parse_name({}, "myArray[2].myVar", value)
        correct_output =  {
                        "myArray": 
                        [   None,
                            None,
                            {"myVar": value}
                        ]
                    }
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_array_index_member_nth_existing_replace(self):
        starting_dict = {'myVar' : 1, "myArray": [1, 2, 3, 4]}
        value = 30
        actual_output = self.driver._parse_name(starting_dict, "myArray[2].myVar", value)
        correct_output =  {
                        "myVar" : 1,
                        "myArray": 
                        [   1,
                            2,
                            {"myVar": value},
                            4
                        ]
                    }
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_structure_array_index_member(self):
        value = 30
        actual_output = self.driver._parse_name({}, "myStruct.myArray[0].myVar", value)
        correct_output =  {
                        "myStruct": 
                        {
                            "myArray": 
                            [
                                {"myVar": value}
                            ]
                        }
                    }
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))


class TestParseName_Complex(omni.kit.test.AsyncTestCase):
    
    # Run before every test
    async def setUp(self):
        self.name_dict = {}
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
        self.test_output_string = "correct: {correct}\nactual: {actual}\n\n"
    
    # Run after every test
    async def tearDown(self):
        pass

    def test_deep_mix_of_nesting(self):
        value = 30
        actual_output = self.driver._parse_name({}, "Task:myStruct.myArray[1].myStruct.arr[3].myVar", value)
        correct_output =  {   "Task": {
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
        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

class TestUtils(omni.kit.test.AsyncTestCase):

    # TODO check accumulation of vars, as well as unique vars

    # Run before every test
    async def setUp(self):
        self.name_dict = {}
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
        self.test_output_string = "correct: {correct}\nactual: {actual}\n\n"
        self.test_different_data_types = ["30", 30, -18935, 30.151535, True, False]
    
    # Run after every test
    async def tearDown(self):
        pass

    def test_variable_name_parsing_simple_struct(self):
        value = "30"
        input = "Program:struct.var"
        correct_output = {
            "Program": {
                "struct": {
                    "var": value}
                }
            }
        
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)

        self.assertEqual(actual_output, 
                         correct_output, 
                         msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_variable_name_parsing_deep_struct(self):
        """Test parsing a variable name with multiple struct levels, with multiple types of values."""
        input = "Program:struct.another_struct.another_struct.var"
        correct_output = {
            "Program": {
                "struct": {
                    "another_struct": {
                        "another_struct": {
                            "var": None
                        }
                    }
                }
            }
        }
        for value in self.test_different_data_types:
            correct_output["Program"]["struct"]["another_struct"]["another_struct"]["var"] = value 

            actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)

            self.assertEqual(actual_output, 
                            correct_output, 
                            msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_variable_name_parsing_simple_array(self):

        value = "30"
        array_size = 5
        input = "Program:myArray[" + str(array_size - 1) + "]"
        correct_output = {
            "Program": {
                "myArray" : [None] * array_size
                }
            }
        correct_output["Program"]["myArray"][array_size - 1] = value
        
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)

        self.assertEqual(actual_output, 
                            correct_output, 
                            msg=self.test_output_string.format(correct=correct_output, actual=actual_output))

    def test_parse_name_with_array(self):
        correct_output = {
            "Task": {
                "myStruct": {
                    "myArray": [
                        {"myVar": "value"}
                    ]
                }
            }
        }

        actual_output = self.driver._parse_name({}, "Task:myStruct.myArray[0].myVar", "value")

        self.assertEqual(actual_output, 
                            correct_output, 
                            msg=self.test_output_string.format(correct=correct_output, actual=actual_output))
