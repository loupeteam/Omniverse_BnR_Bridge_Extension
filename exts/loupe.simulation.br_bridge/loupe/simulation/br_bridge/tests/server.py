# This is a simple Websockets server
# It acts as a mock of a AR instance with OMJSON, responding
# to varaible requests and writing back data with some naievity

# It's intended to be used for initial testing, adding some convenience
# by not having to spin up a whole other simulated PLC

import asyncio
import json
import websockets


import websockets.server

fake_plc_data = [{"LuxProg:counter": 0}]
INITIAL_VALUE_NEW_READ_VAR = 0

async def mock_omjson_plc(websocket, path):
    async for message in websocket:
        response = {
                "type": "readresponse",
                "data": []
        }
        
        print(f"Received message from client: {message}")
        message_dict = json.loads(message)

        if message_dict['type'] == "read":
            for plc_var in message_dict["data"]:
                for plc_var_dict in fake_plc_data: # for every dict in the list
                    if plc_var in plc_var_dict.keys(): # if the requested var is in the keys
                        response["data"].append(plc_var_dict)
                        plc_var_dict[plc_var] = int(plc_var_dict[plc_var]) + 1
                    
            
            await websocket.send(json.dumps(response))

            
        elif message_dict['type'] == "write":
            for plc_var in message_dict["data"]:
                for plc_var_dict in fake_plc_data: # for every dict in the list
                    if plc_var in plc_var_dict.keys(): # if the requested var is in the keys
                        plc_var_dict[plc_var] = message_dict["data"][plc_var]
                        print(fake_plc_data)

            # TODO create writeresponse

start_server = websockets.serve(mock_omjson_plc, "localhost", 8000)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()