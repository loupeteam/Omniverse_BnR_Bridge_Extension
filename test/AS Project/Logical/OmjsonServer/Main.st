
PROGRAM _INIT
  	// pCache is required
	jsonWebSocketServer_0.pCacheArray := ADR(jsonCache);
	jsonWebSocketServer_0.maxClients := SIZEOF(jsonCache)/SIZEOF(jsonCache[0]);
	
	
	// ServerIP, ServerPort, BufferSize, and MaxIterations are optional
  	jsonWebSocketServer_0.ServerIP := '127.0.0.1';
	jsonWebSocketServer_0.ServerPort := 8001;
	//jsonWebSocketServer_0.BufferSize := 1000000;
	//jsonWebSocketServer_0.MaxIterations := 100000;

	// Call jsonWebSocketServer once in the INIT routine to allocate internal memory
	jsonWebSocketServer_0();

END_PROGRAM

PROGRAM _CYCLIC

	// Call jsonWebSocketServer cyclically to serve variables
	jsonWebSocketServer_0();

END_PROGRAM

PROGRAM _EXIT
	
	 
END_PROGRAM

