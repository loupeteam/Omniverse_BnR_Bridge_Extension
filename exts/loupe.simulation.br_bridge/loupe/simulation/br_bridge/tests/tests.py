#from loupe.simulation.br_bridge.websockets_driver import WebsocketsDriver

name_dict = {}
value = "30"
array_size = 30
passes = 0
fails = 0

import omni.kit.test
from loupe.simulation.br_bridge.websockets_driver import WebsocketsDriver

class TestUtils(omni.kit.test.AsyncTestCase):

    # Run before every test
    async def setUp(self):
        self.driver = WebsocketsDriver(ip='127.0.0.1', port=8000)
    
    # Run after every test
    async def tearDown(self):
        pass

    def test_variable_name_parsing_global(self) -> None:

        value = 30
        input = "gBool"
        correct_output = {"gBool": value}
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_simple_struct(self) -> None:

        value = 30
        input = "Program:struct.var"
        correct_output = {"Program": {"struct": {"var": value}}}
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)

    def test_variable_name_parsing_simple_array(self) -> None:

        value = 30
        array_size = 30
        input = "Program:array[30]"
        correct_output = {"Program": {"array" : [value for _ in range(array_size)]}}
        actual_output = self.driver._parse_name(name_dict={}, name=input, value=value)
        self.assertEqual(actual_output, correct_output)#

