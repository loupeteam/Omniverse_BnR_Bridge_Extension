/*
 * File: TCPCommSend.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of TCPComm, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "TCPComm.h"
#include <string.h>

#ifdef __cplusplus
	};
#endif


//***********************************************************
// Cyclic function to send data on an open connection		*
//***********************************************************

// Check input						TESTED
// AcknowledgeError					TESTED
// Reset DataSent, etc.				TESTED
// Close CMD						TESTED

// Main state machine				TESTED
	// ST_CLOSE_CONNECTION			TESTED
	// ST_CLOSED					TESTED
	// ST_RUNNING					TESTED
		// Send FUB					TESTED
		// Detect new connection	TESTED

// Status							TESTED
	// Sending						TESTED
	// DataSent, SentDataLength		TESTED
	// Error						TESTED

unsigned short TCPStreamSend(struct TCPStream_typ* t)
{

	// Check input
	if (t == 0) return TCPCOMM_ERR_INVALIDINPUT;
	
	// AcknowledgeError
	// Errors are treated as informational
	// They do not stop communication
	if (t->IN.CMD.AcknowledgeError) {
		t->OUT.Error = 0;
		t->OUT.ErrorID = 0;
		strcpy(t->OUT.ErrorString, "");
	}

	t->IN.CMD.AcknowledgeError = 0;	

	// Reset DataSent, etc.
	if (!t->IN.CMD.Send || t->IN.PAR.AllowContinuousSend) {
		t->OUT.DataSent = 0;
		t->OUT.SentDataLength = 0;
	}
	
	// Close CMD
	if (t->IN.CMD.Close) {
		t->Internal.Ident = t->IN.PAR.Connection.Ident; // TODO - this is probably not the ident you want to close
		t->Internal.CommState = TCPCOMM_ST_CLOSE_CONNECTION;
	}
	
	t->IN.CMD.Close = 0;
	
	
	// Main state machine
	//--------------------

	switch (t->Internal.CommState) {
		
		// Close an active connection
		//----------------------------
		
		case TCPCOMM_ST_CLOSE_CONNECTION:
			
			// NOTE: functionalize this?
			
			t->OUT.Active = 0;
			
			// Disable send and receive FUBs
			t->Internal.FUB.Send.enable = 0;
			t->Internal.FUB.Receive.enable = 0;
			
			// Close socket
			t->Internal.FUB.Close.enable = 1;
			//t->Internal.FUB.Close.how = (tcpSHUT_RD | tcpSHUT_WR); // NOTE: Check this. I think how should be 0.
			t->Internal.FUB.Close.ident = t->Internal.Ident;
			
			TcpClose(&t->Internal.FUB.Close);
			
			// Wait for socket to close and go to ST_CLOSED
			if (t->Internal.FUB.Close.status < ERR_FUB_ENABLE_FALSE) {
		
				t->Internal.FUB.Close.enable = 0;
				TcpClose(&t->Internal.FUB.Close);
			
				t->Internal.CommState=	TCPCOMM_ST_CLOSED;			
			
				// Fall through for speed
			
			} else {
				// Busy. Stay here.
				break;
			}
			
		
		// Closed state: Connection is inactive
		//--------------------------------------
		
		case TCPCOMM_ST_CLOSED:

			t->OUT.Active = 0;

			// Disable FUBs
			t->Internal.FUB.Send.enable = 0;
			t->Internal.FUB.Receive.enable = 0;
			t->Internal.FUB.Close.enable = 0;
			
			// If there is a valid UID that is not what I have been using, 'connect'
			if (t->IN.PAR.Connection.UID != t->Internal.UID
				&& t->IN.PAR.Connection.UID != 0) {
			
				t->Internal.Ident = t->IN.PAR.Connection.Ident;
				t->Internal.UID = t->IN.PAR.Connection.UID;
				memcpy(&t->OUT.Connection, &t->IN.PAR.Connection, sizeof(t->OUT.Connection));
				t->Internal.CommState = TCPCOMM_ST_RUNNING;
			
				// Fall through for speed
			
			} else {
				break;
			}
		
		
		// Running: Connection is active. Send data.
		//-------------------------------------------

		case TCPCOMM_ST_RUNNING:
			
			t->OUT.Active = 1;

			// Send data
			t->Internal.FUB.Send.enable = (t->IN.CMD.Send || t->OUT.Sending) && (t->IN.PAR.AllowContinuousSend || !t->OUT.DataSent);
			t->Internal.FUB.Send.ident = t->Internal.Ident;
			t->Internal.FUB.Send.pData = t->IN.PAR.pSendData;
			t->Internal.FUB.Send.datalen = t->IN.PAR.SendLength;
			t->Internal.FUB.Send.flags = t->IN.PAR.SendFlags;
		
			// Check for new connection
			if (t->Internal.Ident != t->IN.PAR.Connection.Ident) {
				t->Internal.FUB.Send.enable = 0;
				t->Internal.CommState = TCPCOMM_ST_CLOSE_CONNECTION;
			}
		
			break;
		
		
		default: break;
		
	} // switch(CommState)

	// Call send FUB
	TcpSend(&t->Internal.FUB.Send);

	// Check status
	if (t->Internal.FUB.Send.status == ERR_FUB_BUSY) {
		t->OUT.Sending = 1;
	} else {
		t->OUT.Sending = 0;
	}
		
	if (t->Internal.FUB.Send.status < ERR_FUB_ENABLE_FALSE) {
		if (t->Internal.FUB.Send.status == 0) {
			t->OUT.DataSent = 1;
			t->OUT.SentDataLength = t->Internal.FUB.Send.sentlen;
		} else {
			t->OUT.Error = 1;
			t->OUT.ErrorID = t->Internal.FUB.Send.status;
			strncpy(t->OUT.ErrorString, "Error sending data.", sizeof(t->OUT.ErrorString) - 1);
		}
	}
	
	return 0;

}
