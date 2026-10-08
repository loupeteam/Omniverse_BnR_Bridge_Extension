(*
* File: Main.st
* Copyright (c) 2023 Loupe
* https://loupe.team
* 
* This file is part of OMJSON, licensed under the MIT License.
*)

// Serves PLC variables to OMJSON clients (the Omniverse B&R bridge) over a
// WebSocket on 127.0.0.1:8000.

PROGRAM _INIT

	// pCache is required
	jsonWebSocketServer_0.pCacheArray := ADR(jsonCache);
	jsonWebSocketServer_0.maxClients := MAX_CLIENTS;

	// Loopback only: the bridge runs on the same PC as ARsim
	jsonWebSocketServer_0.ServerIP := '127.0.0.1';
	jsonWebSocketServer_0.ServerPort := 8000;
	jsonWebSocketServer_0.BufferSize := 200000;

	// Call jsonWebSocketServer once in the INIT routine to allocate internal memory
	jsonWebSocketServer_0();

END_PROGRAM

PROGRAM _CYCLIC

	// Call jsonWebSocketServer cyclically to serve variables
	jsonWebSocketServer_0();

END_PROGRAM
