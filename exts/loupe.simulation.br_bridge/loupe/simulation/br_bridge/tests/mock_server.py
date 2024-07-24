'''
  File: **mock_server.py**
  Copyright (c) 2024 Loupe
  https://loupe.team
  
  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.

    Mock OMJSON server
    This is a simple Websockets server.
    It acts as a mock of a AR instance with OMJSON, responding
    to varaible requests and writing back data.

    It's intended to be used for initial testing, adding some convenience
    by not having to spin up a whole other simulated PLC.

    Here are docs on the message formats OMJSON uses:
    https://loupeteam.github.io/LoupeDocs/libraries/omjson/jsonwebsocketserver.html
'''

import asyncio
import json

from websockets.server import serve

# Populate this list of dictionaries with the variables you want to mock
mock_plc_data = [{"TestProg:counter": 0}]

INITIAL_VALUE_NEW_READ_VAR = 0

async def mock_omjson_plc(websocket):
    async for message in websocket:
        response = {
                "type": "readresponse",
                "data": []
        }
        
        #print(f"Received message from client: {message}")
        message_dict = json.loads(message)

        if message_dict['type'] == "read":
            for plc_var in message_dict["data"]:
                for plc_var_dict in mock_plc_data: # for every dict in the list
                    if plc_var in plc_var_dict.keys(): # if the requested var is in the keys
                        response["data"].append(plc_var_dict)
                        # increment the value of the variable every time it's read, just so it changes
                        plc_var_dict[plc_var] = int(plc_var_dict[plc_var]) + 1
                    else:
                        print('not in dict')
            
            await websocket.send(json.dumps(response))

        elif message_dict['type'] == "write":
            for plc_write_var in message_dict["data"].keys():
                for plc_var_dict in mock_plc_data:
                    if plc_write_var in plc_var_dict.keys():
                        plc_var_dict[plc_write_var] = message_dict["data"][plc_write_var]
                    else:
                        print('write failed, not in dict')


async def main():
    async with serve(mock_omjson_plc, "localhost", 8000):
        await asyncio.Future()

asyncio.run(main())