/*
 * File: WSConnection.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of WebSocket, licensed under the MIT License.
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "WebSocket.h"
#include "internal.h"
#include "sha1.h"
#include <string.h>

#ifdef __cplusplus
	};
#endif

//*******************************************************************
// Manage WebSocket connection										*
//*******************************************************************

plcbit wsManageConnection(struct WSConnectionManager_typ* t)
{

	if (t == 0) return 1;
	
	if(t->in.cmd.acknowledgeError) {
		t->out.error = 0;
		t->out.errorID = 0;
		t->out.errorString[0] = '\0';
		t->in.cmd.acknowledgeError = 0;
	}
	
	// Map inputs to internal FUB
	t->internal.tcpConnection.IN.CMD.Enable = t->in.cmd.enable;
	t->internal.tcpConnection.IN.CMD.AcknowledgeConnection = t->in.cmd.acknowledgeConnection;

	// Check for cfg changes
	if(strcmp(t->internal.tcpConnection.IN.CFG.LocalIPAddress, t->in.cfg.localIPAddress) != 0
		|| t->internal.tcpConnection.IN.CFG.LocalPort != t->in.cfg.localPort
		|| strcmp(t->internal.tcpConnection.IN.CFG.RemoteIPAddress, t->in.cfg.remoteIPAddress) != 0
		|| t->internal.tcpConnection.IN.CFG.RemotePort != t->in.cfg.remotePort
		|| (DINT)t->internal.tcpConnection.IN.CFG.Mode != (DINT)t->in.cfg.mode
		|| t->internal.tcpConnection.IN.CFG.UseSSL != t->in.cfg.useSSL
		|| t->internal.tcpConnection.IN.CFG.SendBufferSize != t->in.cfg.sendBufferSize) {
		
		strcpy(t->internal.tcpConnection.IN.CFG.LocalIPAddress, t->in.cfg.localIPAddress);
		t->internal.tcpConnection.IN.CFG.LocalPort = t->in.cfg.localPort;
		t->internal.tcpConnection.IN.CFG.SendBufferSize = t->in.cfg.sendBufferSize;
		t->internal.tcpConnection.IN.CFG.UseSSL = t->in.cfg.useSSL;
		strcpy(t->internal.tcpConnection.IN.CFG.RemoteIPAddress, t->in.cfg.remoteIPAddress);
		t->internal.tcpConnection.IN.CFG.RemotePort = t->in.cfg.remotePort;
		
		// Check for valid mode before copying
		if(!wsModeIsValid(t->in.cfg.mode)) { // Invalid Mode
			// Set error here
			// We do not update internal mode so we keep getting this error until input is fixed
			// No need to throw an error if we are not enabled yet :)
			if(t->in.cmd.enable) {
				internalSetWSConnectionError(t, WS_ERR_INVALID_INPUT, 0);
			}
		}
		else { // Valid Mode
			t->internal.tcpConnection.IN.CFG.Mode = t->in.cfg.mode;
		}
		
		t->internal.tcpConnection.IN.CMD.Enable = 0; // Set enable false so it sees changes in cfg
		
	}
	
	TCPManageConnection(&t->internal.tcpConnection);
	
	// Map outputs from internal FUB
	if(t->internal.tcpConnection.OUT.Error) {
		t->internal.tcpConnection.IN.CMD.AcknowledgeError = 1;
		internalSetWSConnectionError(t, t->internal.tcpConnection.OUT.ErrorID, t->out.errorString);
	}
	
	if (t->internal.tcpConnection.OUT.NewConnectionAvailable) {
		t->out.newConnectionAvailable = 1;
		t->out.connection.mode = t->internal.tcpConnection.IN.CFG.Mode;
		memcpy(&t->out.connection.parameters, &t->internal.tcpConnection.OUT.Connection, sizeof(t->out.connection.parameters));
	}
	else {
		t->out.newConnectionAvailable = 0;
		memset(&t->out.connection, 0, sizeof(t->out.connection));
	}
	
	t->in.cmd.acknowledgeConnection = 0;
	
	return 0;
	
} // End Fn
