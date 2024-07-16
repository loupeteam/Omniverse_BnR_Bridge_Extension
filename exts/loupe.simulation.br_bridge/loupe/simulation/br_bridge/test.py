from websockets_driver import WebsocketsDriver

driver = WebsocketsDriver(ip='127.0.0.1', port=8000)

name_dict = {}
value = "30"
array_size = 30
passes = 0
fails = 0

# first element is input, second is expected output
br_vars = [
    ["Program:struct.var", {"Program": {"struct": {"var": value}}}],
    ["Global", {"Global": value}],
    ["Program:struct.array[30]]", {"Program": {"struct": {"array": [value] * array_size}}}],
]

beckhoff_vars = [
    ["Program.struct.var", {"Program": {"struct": {"var": value}}}],
    ["Global", {"Global": value}],
    ["Program.struct.array[30]]", {"Program": {"struct": {"array": [value] * array_size}}}],
] 
      #  ["Global", } "Global.struct.var", "Program:struct.array[30]", "Program:array[0]", "Program:struct.array[20].struct", "gArray[10]"]

def perform_test(expected, result):
    if expected == result:
        print("PASS")
        global passes
        passes += 1
    else:
        print("FAILLLLLLLL")
        global fails
        fails += 1
        
    print('for var string: ' + str(expected) + '\n' + str(result))
    print('\n\n\n')

def run_all_tests():
    for test_condition in br_vars:
        result = driver._parse_name(name_dict={}, name=test_condition[0], value=value)
        perform_test(expected=test_condition[1], result=result)

    # TODO test dictionary being built
    for test_condition in br_vars:
        result = driver._parse_name(name_dict={}, name=test_condition[0], value=value)
        perform_test(expected=test_condition[1], result=result)

    print('Passes:{}\tFails:{}'.format(passes, fails))

run_all_tests()

