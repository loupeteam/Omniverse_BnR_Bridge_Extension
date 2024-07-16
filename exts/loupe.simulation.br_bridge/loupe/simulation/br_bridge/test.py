from websockets_driver import WebsocketsDriver

driver = WebsocketsDriver(ip='127.0.0.1', port=8000)


name_dict = {}
vars = ["Program:struct.var", "Global", "Global.struct.var", "Program:struct.array[30]", "Program:array[0]", "Program:struct.array[20].struct", "gArray[10]"]
for var in vars:
    print('for var string: ' + var + '\n' + str(driver._parse_name(name_dict={}, name=var, value=30)))
    print('\n\n\n')