/*
 * File: TCPCommManage.c
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


//*******************************************************************
// Cyclic function for managing TCP communications					*
//*******************************************************************
	
// Check input					TESTED
// AcknowledgeConnection		TESTED

// Main state machine			TESTED

	// ST_CLOSED				TESTED
		// With IP				TESTED
		// No IP				TESTED

	// ST_OPENING				TESTED
		// SendBufferSize != 0	TESTED
		// SendBufferSize == 0	TESTED
			// Server			TESTED
			// Client			TESTED
		// Enable FALSE			TESTED
	
	// IOCTL					TESTED
		// Server				TESTED
		// Client				TESTED
		// Enable FALSE			TESTED
	
	// ST_LISTEN				TESTED
		// Server				TESTED
		// Client				not tested - I cannot disable and reenable a client and still connect. Maybe because i am not closing the client's id on the server.
		// Enable FALSE			TESTED

	// ST_CLOSING				TESTED - Can sometimes get wrong ident in weird situations	

	// ST_ERROR					TESTED

// AcknowledgeError				TESTED

// SSL handling					not tested

unsigned short TCPManageConnection(struct TCPConnectionMgr_typ* t)
{

	// Check input
	//-------------

	if (t == 0) return TCPCOMM_ERR_INVALIDINPUT;


	// Handle AcknowledgeConnection
	//------------------------------

	if (t->IN.CMD.AcknowledgeConnection) {
		t->OUT.NewConnectionAvailable = 0;
		memset(&(t->OUT.Connection), 0, sizeof(t->OUT.Connection));
	}

	t->IN.CMD.AcknowledgeConnection = 0;
	

	// Main state machine
	//--------------------

	switch (t->Internal.CommState) {


		case TCPCOMM_ST_CLOSED:
	
			// Reset FUB enables
			t->Internal.FUB.Open.enable = 0;
			t->Internal.FUB.Ioctl.enable = 0;
			t->Internal.FUB.Server.enable = 0;
			t->Internal.FUB.Client.enable = 0;
			t->Internal.FUB.Close.enable = 0;
		
			// Reset status
			memset(&(t->OUT), 0, sizeof(t->OUT));
		
			// Wait for Enable and then open port		
			if (t->IN.CMD.Enable) {
		
				t->Internal.FUB.Open.enable = 1;
			
				if (strcmp(t->IN.CFG.LocalIPAddress, "") == 0) {
					t->Internal.FUB.Open.pIfAddr = 0;
				} else {
					t->Internal.FUB.Open.pIfAddr = (UDINT)&(t->IN.CFG.LocalIPAddress);
				}
			
				t->Internal.FUB.Open.port = t->IN.CFG.LocalPort;
				t->Internal.FUB.Open.options = tcpOPT_REUSEADDR;
			
				t->Internal.CommState = TCPCOMM_ST_OPENING;
				
			}
		
			break;
		
		
		case TCPCOMM_ST_OPENING:
			
			// Wait for port to be open and then either set send buffer size or start listening for connections
			if (t->Internal.FUB.Open.status < ERR_FUB_ENABLE_FALSE) {
		
				t->Internal.FUB.Open.enable = 0;
				
				if ((t->Internal.FUB.Open.status == 0) || (t->Internal.FUB.Open.status == tcpERR_ALREADY_EXIST)) {
					
					// All good, grab ident and move on
					t->Internal.Mode = t->IN.CFG.Mode;
					t->Internal.Ident = t->Internal.FUB.Open.ident;
					
					if (t->IN.CFG.SendBufferSize != 0) {
						
						// If SendBufferSize is configured, set it
						t->Internal.FUB.Ioctl.enable = 1;
						t->Internal.FUB.Ioctl.ident = t->Internal.Ident;
						t->Internal.FUB.Ioctl.ioctl = tcpSO_SNDBUF_SET;
						t->Internal.FUB.Ioctl.pData = (UDINT)&t->IN.CFG.SendBufferSize;
						t->Internal.FUB.Ioctl.datalen = sizeof(t->IN.CFG.SendBufferSize);
					
						t->Internal.CommState = TCPCOMM_ST_IOCTL;
						
					} else {
						
						// Otherwise, move on to Server/Client
						switch (t->Internal.Mode) {
						
							case TCPCOMM_MODE_SERVER:
							
								t->Internal.FUB.Server.ident = t->Internal.FUB.Open.ident;
								t->Internal.FUB.Server.backlog = 3; // NOTE: This 'needs' to be configurable, with a default. maybe...
								t->Internal.FUB.Server.enable = 1;
								t->Internal.FUB.Server.pIpAddr = (UDINT)&(t->OUT.Connection.IPAddress);

								break;

							case TCPCOMM_MODE_CLIENT:

								t->Internal.FUB.Client.ident = t->Internal.FUB.Open.ident;
								t->Internal.FUB.Client.portserv = t->IN.CFG.RemotePort;
								t->Internal.FUB.Client.pServer = (UDINT)&(t->IN.CFG.RemoteIPAddress);
								t->Internal.FUB.Client.enable = 1;
							
								break;

						}

						t->Internal.CommState = TCPCOMM_ST_LISTEN;
						
					}
					
				} else {
			
					// Error. Probably fatal.  Wait for Enable to go false then recover.
					t->OUT.Error = 1;
					t->OUT.ErrorID = t->Internal.FUB.Open.status;
					strncpy(t->OUT.ErrorString, "Error opening port. Check CFG values.", sizeof(t->OUT.ErrorString) - 1);
				
					t->Internal.CommState=	TCPCOMM_ST_ERROR;
				
				} // Error
			
			} else {
				
				// If busy, check for !Enable
				if (!t->IN.CMD.Enable) {
					t->Internal.CommState = TCPCOMM_ST_CLOSING;
				}
				
			}
		
			break;
		
		
		case TCPCOMM_ST_IOCTL:
	
			// Wait for option to be set and then go to ST_LISTEN
			if (t->Internal.FUB.Ioctl.status < ERR_FUB_ENABLE_FALSE) {
		
				t->Internal.FUB.Ioctl.enable = 0;
				
				if (t->Internal.FUB.Ioctl.status == 0) {
					
					switch (t->Internal.Mode) {
						
						case TCPCOMM_MODE_SERVER:
							
							t->Internal.FUB.Server.ident = t->Internal.FUB.Open.ident;
							t->Internal.FUB.Server.backlog = 3; // NOTE: This 'needs' to be configurable, with a default. maybe...
							t->Internal.FUB.Server.enable = 1;
							t->Internal.FUB.Server.pIpAddr = (UDINT)&(t->OUT.Connection.IPAddress);

							break;

						case TCPCOMM_MODE_CLIENT:

							t->Internal.FUB.Client.ident = t->Internal.FUB.Open.ident;
							t->Internal.FUB.Client.portserv = t->IN.CFG.RemotePort;
							t->Internal.FUB.Client.pServer = (UDINT)&(t->IN.CFG.RemoteIPAddress);
							t->Internal.FUB.Client.enable = 1;
							
							break;

					}

					t->Internal.CommState = TCPCOMM_ST_LISTEN;
					
				} else {
			
					// Error. Probably fatal.  Wait for Enable to go false then recover.
					t->OUT.Error = 1;
					t->OUT.ErrorID = t->Internal.FUB.Open.status;
					strncpy(t->OUT.ErrorString, "Error setting option. Check CFG values.", sizeof(t->OUT.ErrorString) - 1);
				
					t->Internal.CommState=	TCPCOMM_ST_ERROR;
				
				} // Error
			
			} else {
				
				// If busy, check for !Enable
				if (!t->IN.CMD.Enable) {
					t->Internal.CommState = TCPCOMM_ST_CLOSING;
				}
			
			}
		
			break;
		
		
		case TCPCOMM_ST_LISTEN:

			switch (t->Internal.Mode) {
				
				case TCPCOMM_MODE_SERVER:

					// Wait for new connection
					// NOTE: Internal.ConnectionUID is only unique to a single TCPCommMgr.
					// This could cause bad behavior if a TCPStream is used with connections from 
					// multiple different TCPCommMgrs. This is not typical but could be an issue.
					// Perhaps we could make the internal UID a library static variable.
					if (t->Internal.FUB.Server.status < ERR_FUB_ENABLE_FALSE) {
		
						if (t->Internal.FUB.Server.status == 0) {
						
							t->Internal.FUB.Server.enable = 0;
							
							t->Internal.ConnectionUID++;
							if (t->Internal.ConnectionUID == 0) t->Internal.ConnectionUID++;

							t->OUT.Connection.Port = t->Internal.FUB.Server.portclnt;
							t->OUT.Connection.Ident = t->Internal.FUB.Server.identclnt;
							t->OUT.Connection.UID = t->Internal.ConnectionUID;
							t->OUT.NewConnectionAvailable = 1;

						} else if (t->Internal.FUB.Server.status != tcpERR_NOCLIENT) {
							// NOTE: No error checking on server. Possible future expansion. Maybe a warning for NOMORE_IDENTS.
						}
						
					} else if (!t->OUT.NewConnectionAvailable) {
						t->Internal.FUB.Server.enable = 1;
					}
					
					break;
				
				case TCPCOMM_MODE_CLIENT:
					
					// Wait for new connection
					if (t->Internal.FUB.Client.status < ERR_FUB_ENABLE_FALSE) {
					
						if (t->Internal.FUB.Client.status == 0) {
							
							t->Internal.FUB.Client.enable = 0;
							
							t->Internal.ConnectionUID++;
							if (t->Internal.ConnectionUID == 0) t->Internal.ConnectionUID++;

							strncpy(t->OUT.Connection.IPAddress, t->IN.CFG.RemoteIPAddress, sizeof(t->OUT.Connection.IPAddress) - 1);
							t->OUT.Connection.Port = t->Internal.FUB.Client.portserv;
							t->OUT.Connection.Ident = t->Internal.FUB.Client.ident;
							t->OUT.Connection.UID = t->Internal.ConnectionUID;
							t->OUT.NewConnectionAvailable = 1;
							
						} else if (t->Internal.FUB.Client.status == tcpERR_INVALID) {
							// Close / ReOpen ident
							t->Internal.CommState = TCPCOMM_ST_CLOSING;	
							// NOTE: The ident (socket) must be closed (TcpClose()) before another attempt can be started.
							// DFB: I am not sure I trust this yet...
						}
						// NOTE: No more error checking on client. Possible future expansion.
						
					} else if(!t->OUT.NewConnectionAvailable) {
						// t->Internal.FUB.Client.enable = 1;
						// NOTE: Why do we need to reenable client? How could we make a new connection here?
					}
					
					break;
				
			}

			if (!t->IN.CMD.Enable) {
				t->Internal.CommState = TCPCOMM_ST_CLOSING;
			}
			
			break;
		
		
		case TCPCOMM_ST_CLOSING:
			
			// Disable other FUBs
			t->Internal.FUB.Open.enable = 0;
			t->Internal.FUB.Ioctl.enable = 0;
			t->Internal.FUB.Server.enable = 0;
			t->Internal.FUB.Client.enable = 0;
				
			t->Internal.FUB.Close.enable = 1;
			t->Internal.FUB.Close.ident = t->Internal.Ident; // NOTE: If you immediately leave ST_OPENING, ident here can be wrong	
			// t->Internal.FUB.Close.how = tcpSHUT_RD | tcpSHUT_WR; // Leave how open for later. I think 0 is correct. We shall see.
			
			// Wait for socket to close and go to ST_CLOSED
			if (t->Internal.FUB.Close.status < ERR_FUB_ENABLE_FALSE) {
		
				t->Internal.FUB.Close.enable = 0;

				t->Internal.Ident = 0;
			
				if (t->Internal.FUB.Close.status != 0) {			
					t->OUT.Error = 1;
					t->OUT.ErrorID = t->Internal.FUB.Close.status;
					strncpy(t->OUT.ErrorString, "Error closing socket", sizeof(t->OUT.ErrorString) - 1);
				}
			
				t->Internal.CommState=	TCPCOMM_ST_CLOSED;			
			
			}
	
			break;
	
		
		case TCPCOMM_ST_ERROR:
	
			// Reset FUB enables
			t->Internal.FUB.Open.enable = 0;
			t->Internal.FUB.Ioctl.enable = 0;
			t->Internal.FUB.Server.enable = 0;
			t->Internal.FUB.Client.enable = 0;
			t->Internal.FUB.Close.enable = 0;
		
			// Wait for Enable to be FALSE
			if (!t->IN.CMD.Enable) {
				t->Internal.CommState = TCPCOMM_ST_CLOSED;
				// NOTE: Should we actually go to CLOSING to close the socket?
				// Right now, the only way to get here is if there is an error on the Open().
				// So, going to CLOSED is probably OK.
				// If we add error checking on Server() and Client(), then we should probably go to CLOSING instead.
			}
		
			break;
		
		
		default: break;
		
	} // switch(CommState) //


	// Call FUBs
	if (t->IN.CFG.UseSSL) {

		// Pipe Open FUB I/O to/from OpenSSL FUB
		// That way you don't have to change any code above here
		t->Internal.FUB.OpenSSL.enable = t->Internal.FUB.Open.enable;
		t->Internal.FUB.OpenSSL.pIfAddr = t->Internal.FUB.Open.pIfAddr;
		t->Internal.FUB.OpenSSL.port = t->Internal.FUB.Open.port;
		t->Internal.FUB.OpenSSL.sslCfgIdent = t->IN.CFG.SSLCertificate;
		t->Internal.FUB.OpenSSL.options = t->Internal.FUB.Open.options;

		TcpOpenSsl(&t->Internal.FUB.OpenSSL);

		t->Internal.FUB.Open.status = t->Internal.FUB.OpenSSL.status;
		t->Internal.FUB.Open.ident = t->Internal.FUB.OpenSSL.ident;		

	} else {

		TcpOpen(&t->Internal.FUB.Open);

	}

	TcpIoctl(&t->Internal.FUB.Ioctl);
	TcpServer(&t->Internal.FUB.Server);
	TcpClient(&t->Internal.FUB.Client);
	TcpClose(&t->Internal.FUB.Close);

	// Handle AcknowledgeError
	if (t->IN.CMD.AcknowledgeError) {
		t->OUT.Error = 0;
		t->OUT.ErrorID = 0;
		strcpy(t->OUT.ErrorString, "");
	}

	t->IN.CMD.AcknowledgeError = 0;

	return 0;

} // End Fn
