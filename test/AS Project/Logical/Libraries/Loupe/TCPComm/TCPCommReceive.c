/*
 * File: TCPCommReceive.c
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
// Cyclic function to receive data on an open connection	*
//***********************************************************

// Check input							TESTED
// AcknowledgeError						TESTED
// Reset DataReceived, etc.				TESTED
// Close CMD							TESTED

// Main state machine					TESTED
	// ST_CLOSE_CONNECTION				TESTED
	// ST_CLOSED						TESTED
	// ST_RUNNING						TESTED
		// Receive FUB					TESTED
		// Detect new connection		TESTED

// Status								TESTED
	// Receiving						TESTED
	// DataReceived, ReceivedDataLength	TESTED
	// Error							TESTED

// Call Recv if done					removed

// Send if busy							TESTED

unsigned short TCPStreamReceive(struct TCPStream_typ* t)
{
	
	// Check input
	if( t == 0 ) return TCPCOMM_ERR_INVALIDINPUT;

	// AcknowledgeError
	// Errors are treated as informational
	// They do not stop communication
	if (t->IN.CMD.AcknowledgeError) {
		t->OUT.Error = 0;
		t->OUT.ErrorID = 0;
		strcpy(t->OUT.ErrorString, "");
	}

	t->IN.CMD.AcknowledgeError = 0;	

	// Reset DataReceived, etc.
	if (!t->IN.CMD.Receive || t->IN.CMD.AcknowledgeData || t->IN.PAR.AllowContinuousReceive) {
		t->OUT.DataReceived = 0;
		t->OUT.ReceivedDataLength = 0;
	}

	t->IN.CMD.AcknowledgeData = 0;

	// Close CMD
	if (t->IN.CMD.Close) {
		t->Internal.Ident = t->IN.PAR.Connection.Ident; // TODO: This is probably not a good ident to use...
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
			
			t->OUT.DataReceived = 0;
			t->OUT.ReceivedDataLength = 0;
			
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
		
		
		// Running: Connection is active. Receive data.
		//----------------------------------------------
		
		case TCPCOMM_ST_RUNNING:

			t->OUT.Active = 1;	

			// Listen for data
			t->Internal.FUB.Receive.enable = t->IN.CMD.Receive && (t->IN.PAR.AllowContinuousReceive || !t->OUT.DataReceived);
			t->Internal.FUB.Receive.ident = t->Internal.Ident;
			t->Internal.FUB.Receive.pData = t->IN.PAR.pReceiveData;
			t->Internal.FUB.Receive.datamax = t->IN.PAR.MaxReceiveLength;
			t->Internal.FUB.Receive.flags = t->IN.PAR.ReceiveFlags;
				
			// Check for new connection
			// TODO: Check for 32609 or recvlen == 0
			if (t->Internal.Ident != t->IN.PAR.Connection.Ident) {
				t->Internal.FUB.Receive.enable = 0;
				t->Internal.CommState = TCPCOMM_ST_CLOSE_CONNECTION;
			}
			
			break;
			
		
		default: break;
		
	} // switch(CommState)

	// Call receive FUB
	memset(&t->Internal.debug.receive.enable, 0, sizeof(t->Internal.debug.receive.enable));
	memset(&t->Internal.debug.receive.status, 0, sizeof(t->Internal.debug.receive.status));
	
	t->Internal.debug.receive.enable[0] = t->Internal.FUB.Receive.enable;
	TcpRecv(&t->Internal.FUB.Receive);
	t->Internal.debug.receive.status[0] = t->Internal.FUB.Receive.status;
	
	
	// Check status
	if ((t->Internal.FUB.Receive.status == ERR_FUB_BUSY) || (t->Internal.FUB.Receive.status == tcpERR_NO_DATA)) {
		t->OUT.Receiving = 1;
	} else {
		t->OUT.Receiving = 0;
	}
	
	// NOTE: Eventually might want to check for 32609, tcpERR_NOT_CONNECTED.
	// If you get that, then the remote side has closed the connection, and you will 
	// need to close this connection. This will cause a loop with a bad ident as
	// the code stands now. I don't want to fix that right now.
	if (t->Internal.FUB.Receive.status < ERR_FUB_ENABLE_FALSE) {

		if (t->Internal.FUB.Receive.status == 0) {
			t->OUT.DataReceived = 1;
			t->OUT.ReceivedDataLength = t->Internal.FUB.Receive.recvlen;
		} else if (t->Internal.FUB.Receive.status == tcpERR_NO_DATA) {
			// No data. Keep listening.
		} else {
			t->OUT.Error = 1;
			t->OUT.ErrorID = t->Internal.FUB.Receive.status;
			strncpy(t->OUT.ErrorString, "Error receiving data.", sizeof(t->OUT.ErrorString) - 1);
		}
		
		// NOTE: Remove this. I am seeing some bad behavior where status is 0 at the end of the scan, which will
		// lead to missed data. BAD!
		// If you are done, call Recv() again so you will be busy and then get new data next scan
		// This can get you new data every scan
//		t->Internal.FUB.Receive.enable = t->IN.CMD.Receive && (t->IN.PAR.AllowContinuousReceive || !t->OUT.DataReceived);
//		
//		t->Internal.debug.receive.enable[1] = t->Internal.FUB.Receive.enable;
//		TcpRecv(&t->Internal.FUB.Receive);
//		t->Internal.debug.receive.status[1] = t->Internal.FUB.Receive.status;

	}
	
	// Call Send() if it is busy to get the most up-to-date 'sending' status.
	// Putting it here allows the scan to finish before calling Send() again.
	// Putting this code inside TCPCommSend() might make more sense at first glance,
	// but that could block execution until the send is finished, which might
	// not be good.
	// TODO: This can cause problems with tcpERR_NOT_CONNECTED
	// If you stop receiving on that error and then try to send,
	// then you can get into a loop where this FUB call is BUSY, which sets
	// Sending, which will cause the next FUB call in TCPCommSend() to 
	// remain enabled
	if (t->Internal.FUB.Send.status == ERR_FUB_BUSY) {
		
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
		
	}
	
	return 0;

}
