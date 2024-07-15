# This is a simple Websockets server
# It acts as a mock of a AR instance with OMJSON, responding
# to varaible requests and writing back data with some naievity

# It's intended to be used for initial testing, adding some convenience
# by not having to spin up a whole other simulated PLC

import asyncio
import websockets
import json

fake_plc_data = {"LuxProg:counter": 0}
INITIAL_VALUE_NEW_READ_VAR = 0

async def echo(websocket, path):
    async for message in websocket:
        response = {
                "type": "readresponse",
                "data": {}
        }
        
        print(f"Received message from client: {message}")
        message_dict = json.loads(message)

        if 'type' in message_dict.keys():
            if message_dict['type'] == "read":
                for plc_var in message_dict["data"]:
                    # If variable doesn't already exist, add it to dictionary
                    # Pretend it exists on the PLC
                    if plc_var not in fake_plc_data.keys():
                        fake_plc_data[plc_var] = INITIAL_VALUE_NEW_READ_VAR

                    response["data"][plc_var] = fake_plc_data[plc_var]
                
                await websocket.send(json.dumps(response))

                fake_plc_data["LuxProg:counter"] += 1

            elif message_dict['type'] == "write":
                print('no')

start_server = websockets.serve(echo, "localhost", 8000)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()