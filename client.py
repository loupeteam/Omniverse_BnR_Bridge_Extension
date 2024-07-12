#!/usr/bin/env python

import asyncio
import websockets
import json

class BrClient():

    def __init__(self, ip, port):
        self._ip = ip
        self._port = str(port)
        self._socket = None
        self._read_list = []
        self._write_dict = {}
        self._data = {}
        self._running = False

    async def connect(self):
        try:
            self._socket = await websockets.connect("ws://" + self._ip + ":" + self._port)
            return True
        except: # TODO specific exceptions
            return False

    def read_cyclically(self, variable):
        if variable not in self._read_list:
            self._read_list.append(variable)

    def write(self, variable, value):
        self._write_dict = { **self._write_dict, **self.set_deep_value(variable, value) }
    
    def set_deep_value(self, variable, value):
        if '.' in variable: 
            split_parts = variable.split('.')
            nested_dict = {split_parts[0]: {}}
            current_level = nested_dict[split_parts[0]]      
            for part in split_parts[1:-1]:
                current_level[part] = {}
                current_level = current_level[part]          
            current_level[split_parts[-1]] = value
            return nested_dict
        else:
            return { variable: value }
                
    async def process_read_request(self, websocket):   
        payload_obj = {
            "type": "read",
            "data": self._read_list
        }
        payload_json = json.dumps(payload_obj)
        await websocket.send(payload_json)
        response_json = await websocket.recv()
        response_obj = json.loads(response_json)
        if response_obj["type"] == "readresponse":
            self.process_read_response(response_obj["data"])
        else:
            print("ERROR! Invalid response received.")

    async def process_write_request(self, websocket):  
        # Only process the object if it's not empty.
        if self._write_dict:
            payload_obj = {
                "type": "write",
                "data": self._write_dict
            }
            payload_json = json.dumps(payload_obj)
            await websocket.send(payload_json)
            self._write_dict = {}
            response_json = await websocket.recv()
            response_obj = json.loads(response_json)
            if response_obj["type"] == "writeresponse":
                self.process_write_response(response_obj["data"])
            else:
                print("ERROR! Invalid response received.")
        else:
            pass

    def process_read_response(self, response):
        self._data = response
        print(self._data)
        return

    def process_write_response(self, response):
        return

async def main():

    client = BrClient(ip="localhost", port=8000)
    client.read_cyclically("LuxProg:counter")
    running = True
    if await client.connect():
        while running:
            await client.process_read_request(client._socket)
            await client.process_write_request(client._socket)
            await asyncio.sleep(.1)
            print('writing')
            client.read_cyclically("LuxProg:counter")

    # client.read_cyclically("LuxProg:structuredCounter")
    # client.write("LuxProg:bonjour", "17")
    #client.write("LuxProg:reset", "1")
    #client.write("LuxProg:structuredCounter.counter1", "36")
asyncio.run(main())