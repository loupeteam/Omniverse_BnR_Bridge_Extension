# server.py
import asyncio
import websockets
import json

fake_plc_data = {"LuxProg:counter": 0}

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
                    if plc_var in fake_plc_data.keys():
                        response["data"][plc_var] = fake_plc_data[plc_var]
                
                await websocket.send(json.dumps(response))

                fake_plc_data["LuxProg:counter"] += 1

            elif message_dict['type'] == "write":
                print('no')

start_server = websockets.serve(echo, "localhost", 8000)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()