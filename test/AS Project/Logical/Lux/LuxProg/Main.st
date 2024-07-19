(*
* File: Main.st
* Copyright (c) 2023 Loupe
* https://loupe.team
* 
* This file is part of OMJSON, licensed under the MIT License.
*)

// This program serves variables to web applications via WebSockets and
// a JSON interface

PROGRAM _INIT
  
    // pCache is required
	jsonWebSocketServer_0.pCacheArray := ADR(jsonCache);
	jsonWebSocketServer_0.maxClients := MAX_CLIENTS;
	
	// Note: ServerIP, ServerPort, BufferSize, and MaxIterations are optional and not set
	jsonWebSocketServer_0.ServerIP := '127.0.0.1';
	jsonWebSocketServer_0.ServerPort := 8000;
	jsonWebSocketServer_0.BufferSize := 200000;

	// Call jsonWebSocketServer once in the INIT routine to allocate internal memory
	jsonWebSocketServer_0();

END_PROGRAM

PROGRAM _CYCLIC

	// Call jsonWebSocketServer cyclically to serve variables
	jsonWebSocketServer_0();
	
	IF NOT SimulationStatus(0) THEN
		gWSTimeout := FALSE;
		FOR i:=0 TO JSON_MAI_CLIENTS DO
			// On any client disconnect
			IF oldClientInfo[i].Connected AND NOT jsonWebSocketServer_0.ClientInfo[i].Connected THEN
				gWSTimeout := TRUE;
			END_IF
		END_FOR
		oldClientInfo := jsonWebSocketServer_0.ClientInfo;
	END_IF
	
END_PROGRAM