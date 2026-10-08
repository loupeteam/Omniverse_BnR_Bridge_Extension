/*
 * File: WSStream.c
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
#include <string.h>
#include "internal.h"
#include "sha1.h"
#include <stdio.h>

#ifdef __cplusplus
	};
#endif

#define UPGRADE_TEMPLATE "GET / HTTP/1.1 \r\nHost: server.example.com\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Origin: http://example.com\r\nSec-WebSocket-Protocol: chat, superchat\r\nSec-WebSocket-Version: 13"

//*******************************************************************
// Manage WebSocket connection										*
//*******************************************************************

void webSocketOnDisconnect(struct WSStream_typ* t) {
	t->internal.fub.tcpStream.IN.CMD.Receive = 0;
	t->internal.fub.tcpStream.IN.CMD.Send = 0;
	t->internal.fub.tcpStream.IN.CMD.Close = 1;
		
	t->internal.connected = 0;
	t->internal.connectionUpgraded = 0;
	t->internal.connectionState = 0;
}

void webSocketShiftReceivePointer(struct WSStream_typ* t, unsigned long bytes) {
	if(t->internal.fub.tcpStream.IN.PAR.MaxReceiveLength < bytes) {
		t->internal.fub.tcpStream.IN.PAR.pReceiveData += t->internal.fub.tcpStream.IN.PAR.MaxReceiveLength;
		t->internal.fub.tcpStream.IN.PAR.MaxReceiveLength = 0;
		
		internalSetWSStreamError(t, WS_ERR_BUFFER_FULL, 0);
		t->internal.debug.receiveBufferShiftFull++;
	}
	else {
		t->internal.fub.tcpStream.IN.PAR.pReceiveData += bytes;
		t->internal.fub.tcpStream.IN.PAR.MaxReceiveLength -= bytes;
	}
	t->internal.debug.receiveBufferShift++;
	t->internal.debug.receiveBufferIsShifted = 1;
}
void webSocketResetReceivePointer(struct WSStream_typ* t) {
	t->internal.fub.tcpStream.IN.PAR.pReceiveData = t->internal.receiveBuffer;
	t->internal.fub.tcpStream.IN.PAR.MaxReceiveLength = t->internal.bufferSize;
	t->internal.debug.receiveBufferReset++;
	t->internal.debug.receiveBufferIsShifted = 0;
}

plcbit webSocketStreamInitialize(struct WSStream_typ* t) {
	
	t->internal.bufferSize = t->in.cfg.bufferSize + WS_HEADER_MAX_LEN;
	
	if(t->internal.receiveBuffer == 0)
		if(TMP_alloc(t->internal.bufferSize, (void**)&t->internal.receiveBuffer) != 0) {
			internalSetWSStreamError(t, WS_ERR_MEM_ALLOC, 0);
		}
	
	if(t->internal.sendBuffer == 0)
		if(TMP_alloc(t->internal.bufferSize, (void**)&t->internal.sendBuffer) != 0) {
			internalSetWSStreamError(t, WS_ERR_MEM_ALLOC, 0);
		}
	
	
	t->internal.fub.tcpStream.IN.PAR.pReceiveData = t->internal.receiveBuffer;
	t->internal.fub.tcpStream.IN.PAR.MaxReceiveLength = t->internal.bufferSize-1;
	t->internal.fub.tcpStream.IN.PAR.AllowContinuousSend = 1;
	t->internal.fub.tcpStream.IN.PAR.pSendData = t->internal.sendBuffer;
	
	t->internal.initialized = (t->internal.receiveBuffer) && (t->internal.sendBuffer);
	
	return t->internal.initialized;
}

plcbit wsSend(struct WSStream_typ* t)
{
	if(!t) return 1;
	
	if(t->in.cmd.acknowledgeError) {
		t->out.error = 0;
		t->out.errorID = 0;
		t->out.errorString[0] = '\0';
		t->in.cmd.acknowledgeError = 0;
	}
	
	if(!t->internal.initialized) {
		webSocketStreamInitialize(t); // This fn will set errors directly if they occur
		if(!t->internal.initialized) return 1; // If we failed to initiallize, don't continue as it can cause page faults
	}
	
	if(memcmp(&t->internal.connection, &t->in.par.connection, sizeof(t->internal.connection)) != 0) {
		if(t->internal.connected) {
			webSocketOnDisconnect(t); // Handle disconnect
			TCPStreamSend(&t->internal.fub.tcpStream); // TcpStream uses in.par.ident to close not internal so we need to call it before reconnecting
		}
		
		// Handle connect
		t->internal.connected = 1; 
		webSocketResetReceivePointer(t);
		
		// Copy connection parameters
		memcpy(&t->internal.connection, &t->in.par.connection.parameters, sizeof(t->internal.connection));
		memcpy(&t->internal.fub.tcpStream.IN.PAR.Connection, &t->in.par.connection.parameters, sizeof(t->internal.fub.tcpStream.IN.PAR.Connection));
		memcpy(&t->out.connection, &t->in.par.connection.parameters, sizeof(t->out.connection));
	}
	
	if(!t->internal.connectionUpgraded) {
		// Upgrade http(s) connection to a ws(s)
		// Client request upgrade, Server accepts
		if(t->internal.connection.mode == WS_MODE_SERVER) {
			// Server is required to acknowledge the WS upgrade request
			// Do nothing here as the send command will be set in the webSocketRecv fn after recieving the request
			t->internal.connectionUpgraded = t->internal.fub.tcpStream.IN.CMD.Send;
		}
		else if(t->internal.connection.mode == WS_MODE_CLIENT) {
			if(t->internal.connectionState == 0) {
				char tmpString[24];
				// As a client we send the upgrade request
				// TODO: build
				//sprintf(t->internal.sendBuffer, "");
			
			
				strcpy((char*)t->internal.sendBuffer, "GET / HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nHost: ");
				strcat((char*)t->internal.sendBuffer, t->internal.connection.parameters.IPAddress);
				strcat((char*)t->internal.sendBuffer, ":");
				brsitoa(t->internal.connection.parameters.Port, tmpString);
				strcat((char*)t->internal.sendBuffer, tmpString);
				strcat((char*)t->internal.sendBuffer, "\r\nSec-WebSocket-Key: ");
				strcat((char*)t->internal.sendBuffer, "dBQ0epP7wmgHDEJc6Me95Q==");
				strcat((char*)t->internal.sendBuffer, "\r\nSec-WebSocket-Version: 13\r\n");
				//strcat((char*)t->internal.sendBuffer, "\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: chat\r\n");
			
				//"Sec-WebSocket-Extensions: x-webkit-deflate-frame\r\n"
			
				strcat((char*)t->internal.sendBuffer, "\r\n"); // End header
			
				t->internal.fub.tcpStream.IN.CMD.Send = 1;
				t->internal.fub.tcpStream.IN.PAR.SendLength = strlen((char*)t->internal.sendBuffer);
				
				t->internal.connectionState = 1;
			}
			else {
				// If Recieve a packet, then say we good 
				if(t->internal.fub.tcpStream.OUT.DataReceived) {
					t->internal.connectionUpgraded = 1;
					if(!t->in.par.allowContinuousReceive) {
						t->internal.fub.tcpStream.IN.CMD.AcknowledgeData = 1;
					}
				}
			}
			
			// NOT IMPLEMENTED YET
			//internalSetWSStreamError(t, WS_ERR_NOT_IMPLEMENTED, 0);
		}
	}
	else if(t->in.cmd.send && (!t->internal.prevSend || t->in.par.allowContinuousSend)) {
		// TODO: Make this more configurable 
		t->internal.fub.wsEncode.fin = 1;
		t->internal.fub.wsEncode.mask = t->internal.connection.mode == WS_MODE_CLIENT; // Clients require masking
		t->internal.fub.wsEncode.opCode = 1;
		t->internal.fub.wsEncode.rsv = 0;
		
		t->internal.fub.wsEncode.payloadLength = t->in.par.sendLength;
		t->internal.fub.wsEncode.pPayload = t->in.par.pSendData;
		
		t->internal.fub.wsEncode.frameSize = t->internal.bufferSize;
		t->internal.fub.wsEncode.pFrame = t->internal.sendBuffer;
		
		wsEncode(&t->internal.fub.wsEncode);
		
		if(t->internal.fub.wsEncode.status != 0) {
			// Handle error	
			internalSetWSStreamError(t, t->internal.fub.wsEncode.status, 0);
		}
		else {
			// Send encoded data
			t->internal.fub.tcpStream.IN.CMD.Send = 1;
			t->internal.fub.tcpStream.IN.PAR.pSendData = t->internal.sendBuffer;
			t->internal.fub.tcpStream.IN.PAR.SendLength = t->internal.fub.wsEncode.frameLength;
		}
	}
	t->internal.prevSend = t->in.cmd.send;
	
	if(t->in.cmd.close) {
		t->in.cmd.close = 0;
		webSocketOnDisconnect(t);
	}
	
	t->internal.fub.tcpStream.IN.PAR.SendFlags = t->in.par.sendFlags;
	TCPStreamSend(&t->internal.fub.tcpStream);
	t->internal.fub.tcpStream.IN.CMD.Send = 0;
	t->internal.fub.tcpStream.IN.CMD.AcknowledgeData = 0;
	t->internal.fub.tcpStream.IN.CMD.Close = 0;
	t->internal.fub.tcpStream.IN.CMD.AcknowledgeError = 0;
	
	t->out.connected = t->internal.connected && t->internal.connectionUpgraded;
	t->out.active = t->internal.connected;
	t->out.receiving = t->out.connected && t->internal.fub.tcpStream.OUT.Receiving; // Dont show recieving if we have not upgraded the connection
	t->out.sending = t->out.connected && t->internal.fub.tcpStream.OUT.Sending;
	t->out.dataSent = t->out.connected && t->internal.fub.tcpStream.OUT.DataSent;
	t->out.sentDataLength = t->out.connected ? t->internal.fub.tcpStream.OUT.SentDataLength : 0; // TODO: Should this be sendDataLenght minus header?
	
	return 0;
	
}

plcbit wsReceive(struct WSStream_typ* t)
{
	if(!t) return 1;
	
	if(t->in.cmd.acknowledgeError) {
		t->out.error = 0;
		t->out.errorID = 0;
		t->out.errorString[0] = '\0';
		t->in.cmd.acknowledgeError = 0;
	}
	
	if(!t->internal.initialized) {
		webSocketStreamInitialize(t); // This fn will set errors directly if they occur
		if(!t->internal.initialized) return 1; // If we failed to initiallize dont continue as it can cause page faults
	}

	if(!t->internal.connected) {
		// Do nothing yet but maybe something here in the future
	}
	else if(t->in.cmd.receive || !t->internal.connectionUpgraded) {
		t->internal.fub.tcpStream.IN.CMD.Receive = 1;
	}
	

	// Check for connection parameter changes
	if(memcmp(&t->internal.connection, &t->in.par.connection, sizeof(t->internal.connection)) != 0) {
		if(t->internal.connected) {
			webSocketOnDisconnect(t); // Handle disconnect
			TCPStreamSend(&t->internal.fub.tcpStream); // TcpStream uses in.par.ident to close not internal so we need to call it before reconnecting
		}
		
		// Handle connect
		t->internal.connected = 1; 
		webSocketResetReceivePointer(t);
		
		// Copy connection parameters
		memcpy(&t->internal.connection, &t->in.par.connection, sizeof(t->internal.connection));
		memcpy(&t->internal.fub.tcpStream.IN.PAR.Connection, &t->in.par.connection.parameters, sizeof(t->internal.fub.tcpStream.IN.PAR.Connection));
		memcpy(&t->out.connection, &t->in.par.connection, sizeof(t->out.connection));
	}
	
	if(t->in.cmd.close) {
		t->in.cmd.close = 0;
		webSocketOnDisconnect(t);
	}
	
	t->internal.fub.tcpStream.IN.PAR.AllowContinuousReceive = t->in.par.allowContinuousReceive;
	t->internal.fub.tcpStream.IN.PAR.ReceiveFlags = t->in.par.receiveFlags;
	t->internal.fub.tcpStream.IN.CMD.AcknowledgeData |= t->in.cmd.acknowledgeData; // Set acknowledge data from user. It may also be set internally so dont override it
	TCPStreamReceive(&t->internal.fub.tcpStream);
	t->internal.fub.tcpStream.IN.CMD.Receive = 0;
	t->internal.fub.tcpStream.IN.CMD.AcknowledgeData = 0;
	t->internal.fub.tcpStream.IN.CMD.Close = 0;
	t->internal.fub.tcpStream.IN.CMD.AcknowledgeError = 0;
	
	if(t->in.par.allowContinuousReceive || t->in.cmd.acknowledgeData) {
		t->out.dataReceived = 0; // Set output here, it will get overwritten if data is recieved below
		t->out.receivedDataLength = 0;
		memset(&t->out.header, 0, sizeof(t->out.header)); 
	}
	
	if(t->internal.fub.tcpStream.OUT.Error) {
		switch (t->internal.fub.tcpStream.OUT.ErrorID) {
			case tcpERR_NOT_CONNECTED:
			case tcpERR_INVALID_IDENT:
				// Handle errors
				// These errors will be handled and not shown to the user
				t->internal.fub.tcpStream.IN.CMD.AcknowledgeError = 1;
				
				webSocketOnDisconnect(t);

				break;

			default:
				// Unexpected errors
				t->internal.fub.tcpStream.IN.CMD.AcknowledgeError = 1;
				
				webSocketOnDisconnect(t);
				
				// Pass them through
				internalSetWSStreamError(t, t->internal.fub.tcpStream.OUT.ErrorID, t->internal.fub.tcpStream.OUT.ErrorString);
				
				break;
		}
     	
	}
	else if(t->internal.fub.tcpStream.OUT.DataReceived && t->internal.fub.tcpStream.OUT.ReceivedDataLength == 0) {
		// Connection closed by client
		webSocketOnDisconnect(t);
	}
	else if(!t->internal.connectionUpgraded && t->internal.fub.tcpStream.OUT.DataReceived) {
		// Upgrade http(s) connection to a ws(s)
		// Client request upgrade, Server accepts
		if(t->internal.connection.mode == WS_MODE_SERVER) {
			if(!t->in.par.allowContinuousReceive) {
				t->internal.fub.tcpStream.IN.CMD.AcknowledgeData = 1;
			}

			t->internal.fub.wsConnect.outputMessageSize = t->internal.bufferSize;
			t->internal.fub.wsConnect.pInputMessage = t->internal.fub.tcpStream.IN.PAR.pReceiveData;
			t->internal.fub.wsConnect.pOutputMessage = t->internal.sendBuffer;
			wsConnect(&t->internal.fub.wsConnect);
			
			if(t->internal.fub.wsConnect.status == WS_ERR_PARTIAL_HTTP_MESSAGE) {
				// Shift the point to append the new data to current message
				webSocketShiftReceivePointer(t, t->internal.fub.tcpStream.OUT.ReceivedDataLength);
			}
			else if(t->internal.fub.wsConnect.status != 0) {
				// TODO: Handle errors
				internalSetWSStreamError(t, t->internal.fub.wsConnect.status, 0);
				webSocketResetReceivePointer(t);
			}
			else {
				webSocketResetReceivePointer(t);
				t->internal.fub.tcpStream.IN.PAR.pSendData = t->internal.sendBuffer;
				t->internal.fub.tcpStream.IN.PAR.SendLength = t->internal.fub.wsConnect.outputMessageLength;
				t->internal.fub.tcpStream.IN.CMD.Send = 1;
				//t->internal.connectionUpgraded = 1; // Lets try to set this here even though we have not sent the message yet
			}
		}
		else if(t->internal.connection.mode == WS_MODE_CLIENT) {
			// We are handling connection in send
		}
	}
	else if(t->internal.fub.tcpStream.OUT.DataReceived) {
		t->internal.fub.wsDecode.pFrame = t->internal.fub.tcpStream.IN.PAR.pReceiveData;
		t->internal.fub.wsDecode.frameLength = t->internal.fub.tcpStream.OUT.ReceivedDataLength;
		t->internal.fub.wsDecode.pPayload = t->in.par.pReceiveData;
		t->internal.fub.wsDecode.payloadSize = t->in.par.maxReceiveLength;
		wsDecode(&t->internal.fub.wsDecode);
		
		if(t->internal.fub.wsDecode.status) {
			internalSetWSStreamError(t, t->internal.fub.wsDecode.status, 0);
			
			if(!t->in.par.allowContinuousReceive) {
				t->internal.fub.tcpStream.IN.CMD.AcknowledgeData = 1; // Acknowledge data as we have a bad packet
			}
			
			// If we get an error on a packet then dont keey building the frame
			webSocketResetReceivePointer(t);
			
		}
		else if(t->internal.fub.wsDecode.opCode == WS_OPCODE_CONNECTION_CLOSE) {
			// Lets close gracefully
			webSocketOnDisconnect(t);
		}
		else {
			
			if(t->internal.fub.wsDecode.decodeLength < t->internal.fub.tcpStream.OUT.ReceivedDataLength) {
				t->internal.debug.websocketPacketTooBig++;
			}
		
			t->out.partialDataReceived = t->internal.fub.wsDecode.partialFrame || t->internal.fub.wsDecode.partialHeader;
			t->out.header.fin = t->internal.fub.wsDecode.fin;
			t->out.header.mask = t->internal.fub.wsDecode.mask;
			t->out.header.rsv = t->internal.fub.wsDecode.rsv;
			t->out.header.opCode = t->internal.fub.wsDecode.opCode;
			t->out.header.frameLength = t->internal.fub.wsDecode.decodeLength;
			//t->out.header.payloadLength = t->internal.fub.wsDecode.PayloadLength;
			//t->out.header.pPayloadData = t->internal.fub.wsDecode.pPayloadData;
			
			if(!t->internal.fub.wsDecode.partialFrame) {
			
				if(t->internal.fub.wsDecode.decodeLength < t->internal.fub.tcpStream.OUT.ReceivedDataLength) {
					// TODO: We should handle multiple packets better
					// If we got another packet. Lets see if its a close message
					USINT *pFrame = (USINT*)t->internal.fub.tcpStream.IN.PAR.pReceiveData + t->internal.fub.wsDecode.decodeLength;
					USINT opCode = (*pFrame & 0x0f);
					if(opCode == WS_OPCODE_CONNECTION_CLOSE) {
						// Lets close gracefully
						webSocketOnDisconnect(t);
					}
				}
				
				// We recieved a whole frame
				webSocketResetReceivePointer(t); 
			
				//				if(t->internal.fub.wsDecode.mask && t->in.par.pReceiveData) {
				//					t->internal.fub.wsMask.pDest = t->in.par.pReceiveData;
				//					t->internal.fub.wsMask.destSize = t->in.par.maxReceiveLength;
				//					t->internal.fub.wsMask.pSrc = t->internal.fub.wsDecode.pPayloadData;
				//					t->internal.fub.wsMask.srcLength = t->internal.fub.wsDecode.payloadLength;
				//					
				//					memcpy(t->internal.fub.wsMask.maskingKey, t->internal.fub.wsDecode.maskingKey, sizeof(t->internal.fub.wsMask.maskingKey));
				//					
				//					WSMask(&t->internal.fub.wsMask);
				//					
				//					if(t->internal.fub.wsMask.status != 0) {
				//						internalSetWSStreamError(t, t->internal.fub.wsMask.status, 0);	
				//					}
				//				}
				//				else if(t->in.par.pReceiveData) {
				//					// TODO: Handle pRecieveData not being large enough
				//					memcpy((void*)t->in.par.pReceiveData, (void*)t->internal.fub.wsDecode.pPayloadData, t->internal.fub.wsDecode.payloadLength);
				//				}
			
				t->out.dataReceived = 1;
				t->out.receivedDataLength = t->internal.fub.wsDecode.payloadLength;
			}
			else {
				if(!t->in.par.allowContinuousReceive) {
					t->internal.fub.tcpStream.IN.CMD.AcknowledgeData = 1; // Acknowledge data so we can get rest of packet
				}
				
				webSocketShiftReceivePointer(t, t->internal.fub.tcpStream.OUT.ReceivedDataLength); // Shift recieve pointer so we can append new data
				
				//				t->out.dataReceived = 0;
				//				t->out.receivedDataLength = 0;
			}
		}
	}
	
	t->out.connected = t->internal.connected && t->internal.connectionUpgraded;
	t->out.active = t->internal.connected;
	t->out.receiving = t->out.connected && t->internal.fub.tcpStream.OUT.Receiving; // Dont show recieving if we have not upgraded the connection
	t->out.sending = t->out.connected && t->internal.fub.tcpStream.OUT.Sending;
	t->out.dataSent = t->out.connected && t->internal.fub.tcpStream.OUT.DataSent;
	t->out.sentDataLength = t->out.connected ? t->internal.fub.tcpStream.OUT.SentDataLength : 0; // TODO: Should this be sendDataLenght minus header?
	
	return 0;

}
