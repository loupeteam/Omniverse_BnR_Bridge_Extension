#from loupe.simulation.br_bridge.websockets_driver import WebsocketsDriver

name_dict = {}
value = "30"
array_size = 30
passes = 0
fails = 0

import omni.kit.test
import loupe.simulation.br_bridge

class TestUtils(omni.kit.test.AsyncTestCase):

    # Run before every test
    async def setUp(self):
        print('setUp')
    
    # Run after every test
    async def tearDown(self):
        print('tearDown')

    def test_basic(self):
        print('test basic running')
        self.assertTrue(True)

    def test_basi2c(self):
        print('test basic running')
        self.assertTrue(True)

    # def test_variable_name_parsing_global(self) -> None:

    #     driver = WebsocketsDriver(ip='127.0.0.1', port=8000)

    #     value = 30
    #     input = "Program:struct.var"
    #     correct_output = {"Program": {"struct": {"var": value}}}
    #     actual_output = driver._parse_name(name_dict={}, name=input, value=value)
    #     self.assertEqual(actual_output, correct_output)

#     def test_variable_structured(self) -> None:


# # first element is input, second is expected output
# br_vars = [
    
#     ["Global", {"Global": value}],
#     ["Program:struct.array[30]]", {"Program": {"struct": {"array": [value] * array_size}}}],
# ]

# beckhoff_vars = [
#     ["Program.struct.var", {"Program": {"struct": {"var": value}}}],
#     ["Global", {"Global": value}],
#     ["Program.struct.array[30]]", {"Program": {"struct": {"array": [value] * array_size}}}],
# ] 
#       #  ["Global", } "Global.struct.var", "Program:struct.array[30]", "Program:array[0]", "Program:struct.array[20].struct", "gArray[10]"]

# def perform_test(expected, result):
#     if expected == result:
#         print("PASS")
#         global passes
#         passes += 1
#     else:
#         print("FAILLLLLLLL")
#         global fails
#         fails += 1

#     print('for var string: ' + str(expected) + '\n' + str(result))
#     print('\n\n\n')

# def run_all_tests():
#     for test_condition in br_vars:
#         result = driver._parse_name(name_dict={}, name=test_condition[0], value=value)
#         perform_test(expected=test_condition[1], result=result)

#     # TODO test dictionary being built
#     for test_condition in br_vars:
#         result = driver._parse_name(name_dict={}, name=test_condition[0], value=value)
#         perform_test(expected=test_condition[1], result=result)

#     print('Passes:{}\tFails:{}'.format(passes, fails))

# run_all_tests()

