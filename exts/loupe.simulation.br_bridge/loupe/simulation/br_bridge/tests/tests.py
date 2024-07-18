name_dict = {}
value = "30"
array_size = 30
passes = 0
fails = 0

import omni.kit.test
from loupe.simulation.br_bridge.websockets_driver import WebsocketsDriver

class TestUtils(omni.kit.test.AsyncTestCase):

    # TODO check accumulation of vars, as well as unique vars

    # Run before every test
    async def setUp(self):
        self.name_dict = {}
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
    
    # Run after every test
    async def tearDown(self):
        pass

    def test_variable_name_parsing_global(self) -> None:
        value = True
        input = "gBool"
        correct_output = {
            input: value
        }
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

        value = -3623
        input = "gInt"
        correct_output = {
            input: value
        }
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)


    def test_variable_name_parsing_simple_struct(self) -> None:

        value = "30"
        input = "Program:struct.var"
        correct_output = {
            "Program": {
                "struct": {
                    "var": value}
                }
            }
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_deep_struct(self) -> None:

        value = "30"
        input = "Program:struct.another_struct.another_struct.var"
        correct_output = {
            "Program": {
                "struct": {
                    "another_struct": {
                        "another_struct": {
                            "var": value
                        }
                    }
                }
            }
        }
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

        value = 30
        correct_output["Program"]["struct"]["another_struct"]["another_struct"]["var"] = value 
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

        value = 30.151535
        correct_output["Program"]["struct"]["another_struct"]["another_struct"]["var"] = value 
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_simple_array(self) -> None:

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
        msg = "\n" + str(correct_output) + "\n" + str(actual_output)
        self.assertEqual(actual_output, correct_output, msg=msg)

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
        msg = "\n" + str(correct_output) + "\n" + str(actual_output)
        self.assertEqual(actual_output, correct_output, msg=msg)

