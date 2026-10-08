/*
 * File: jsonHTTPServer.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of OMJSON, licensed under the MIT License.
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "OMJSON.h"
#include <string.h>
#include "jsonAux.h"
#include "jsonInternal.h"

#ifdef __cplusplus
	};
#endif


//*******************************************************************
// Serve variables via JSON and HTTP 								*
//*******************************************************************

void jsonHTTPServer(struct jsonHTTPServer* t)
{


	// Check for null pointer //

	if( t == 0 ) return;


	//***********************************
	// Acknowledge errors				*
	//***********************************

	if( t->AcknowledgeError ){
		t->AcknowledgeError = 0;
		t->Error = 0;
		t->ErrorID = 0;
		memset( &(t->ErrorString), 0, sizeof(t->ErrorString) );
	} // AcknowledgeError


	//***************************************************
	// Initialize 										*
	//***************************************************

	if( !t->internal.initialized ){
		if( t->Error ){
			return;
		} 			
		else {
		
			// Check defaults
			if( t->BufferSize == 0 ) t->BufferSize = JSON_DEFAULT_BUFFERSIZE;
			if( t->MaxIterations == 0 ) t->MaxIterations = JSON_DEFAULT_MAXITERATIONS;
			
			// Allocate memory
			if(		(TMP_alloc( t->BufferSize, (void**)&(t->internal.read.pUri) ) != 0)
				||	(TMP_alloc( t->BufferSize, (void**)&(t->internal.read.pRequestData) ) != 0)				
				
				||	(TMP_alloc( t->BufferSize, (void**)&(t->internal.write.pUri) ) != 0)
				||	(TMP_alloc( t->BufferSize, (void**)&(t->internal.write.pRequestData) ) != 0)
			){
				jsonInternalSetHTTPServerError( JSON_ERR_MEMALLOC, t );
				return;
			} // Error allocating memory
			
			// Set header options
			strcpy(t->internal.read.ResponseHeader.connection, "keep-alive");
			strcpy(t->internal.read.ResponseHeader.keepAlive, "timeout=5000, max=10");			
			
			strcpy(t->internal.write.ResponseHeader.connection, "keep-alive");
			strcpy(t->internal.write.ResponseHeader.keepAlive, "timeout=5000, max=10");			
			
			// Set up services
		
			// Read
		
			t->internal.read.webService.enable = 1;
		
			t->internal.read.webService.option = httpOPTION_HTTP_11 + httpOPTION_SERVICE_TYPE_NAME;

			STRING readServiceName[JSON_STRLEN_SERVICENAMEPREFIX + 8 + 1];
			memset(readServiceName, 0, sizeof(readServiceName));			
			strcpy(readServiceName, t->ServiceNamePrefix);
			strcat(readServiceName, "read.cgi");
			
			t->internal.read.webService.pServiceName = (UDINT)&readServiceName;
		
			t->internal.read.webService.pUri = t->internal.read.pUri;
			t->internal.read.webService.uriSize = t->BufferSize;
			
			t->internal.read.webService.pRequestData = t->internal.read.pRequestData;
			t->internal.read.webService.requestDataSize = t->BufferSize;
				
			t->internal.read.webService.pResponseHeader= (UDINT)&(t->internal.read.ResponseHeader);
			
			httpService( &(t->internal.read.webService) );
		
		
			// Write
		
			t->internal.write.webService.enable = 1;
		
			t->internal.write.webService.option = httpOPTION_HTTP_11 + httpOPTION_SERVICE_TYPE_NAME;
		
			STRING writeServiceName[JSON_STRLEN_SERVICENAMEPREFIX + 9 + 1];
			memset(writeServiceName, 0, sizeof(writeServiceName));			
			strcpy(writeServiceName, t->ServiceNamePrefix);
			strcat(writeServiceName, "write.cgi");
			
			t->internal.write.webService.pServiceName = (UDINT)&writeServiceName;
		
			t->internal.write.webService.pUri = t->internal.write.pUri;
			t->internal.write.webService.uriSize = t->BufferSize;
		
			t->internal.write.webService.pRequestData = t->internal.write.pRequestData;
			t->internal.write.webService.requestDataSize = t->BufferSize;

			t->internal.write.webService.pResponseHeader= (UDINT)&(t->internal.write.ResponseHeader);

			httpService( &(t->internal.write.webService) );
		
			
			// DONE
		
			t->internal.initialized = 1;
		
			return;
		
		} // !error

	}// if(!initialized)


	//***************************************************
	// Read		 										*
	//***************************************************

	if( 	(t->internal.read.webService.phase == httpPHASE_RECEIVED)
		&&	!(t->internal.read.processed)
	){
		
		// Read variable
		t->internal.read.readVariableList.pVariableList = t->internal.read.pRequestData;
		t->internal.read.readVariableList.pCache = t->pCache;
		t->internal.read.readVariableList.BufferSize = t->BufferSize;
		t->internal.read.readVariableList.MaxIterations = t->MaxIterations;
		
		jsonReadVariableList( &(t->internal.read.readVariableList) );
		
		if( t->internal.read.readVariableList.Status == 0 ){
			// Send response
			t->internal.read.webService.pResponseData = t->internal.read.readVariableList.pJSONObject;
			t->internal.read.webService.responseDataLen = t->internal.read.readVariableList.JSONObjectLength;
			t->internal.read.webService.send = 1;	
		}
		else{
			// ERROR!
			jsonInternalSetHTTPServerError( t->internal.read.readVariableList.Status, t );
			t->internal.read.webService.abort = 1;
		}

		t->internal.read.processed = 1;	

	} // if(RECEIVED and not processed) //


	if( t->internal.read.webService.phase != httpPHASE_RECEIVED ) t->internal.read.processed = 0;
	
	//t->internal.debug.readSendCommand = t->internal.read.webService.send;

	httpService( &(t->internal.read.webService) );

	t->internal.read.webService.send = 0;
	t->internal.read.webService.abort = 0;


	//***************************************************
	// Write		 									*
	//***************************************************

	if( 	(t->internal.write.webService.phase == httpPHASE_RECEIVED)
		&&	!(t->internal.write.processed)
	){

		// Write variable
		t->internal.write.writeVariable.pJSONObject = t->internal.write.pRequestData;
		t->internal.write.writeVariable.MaxJSONObjectLength = t->BufferSize;
		t->internal.write.writeVariable.MaxIterations = t->MaxIterations;
		
		jsonWriteVariable( &(t->internal.write.writeVariable) );
		
		if( t->internal.write.writeVariable.Status == 0 ){
			// Send response - what to send??? echo???
			t->internal.write.webService.pResponseData = t->internal.write.pRequestData;
			t->internal.write.webService.responseDataLen = strlen((char*)t->internal.write.pRequestData);
			t->internal.write.webService.send = 1;	
		}
		else{
			// ERROR!
			jsonInternalSetHTTPServerError( t->internal.write.writeVariable.Status, t );
			t->internal.write.webService.abort = 1;
		}

		t->internal.write.processed = 1;	

	} // if(RECEIVED) //


	if( t->internal.write.webService.phase != httpPHASE_RECEIVED ) t->internal.write.processed = 0;
	
	//t->internal.debug.writeSendCommand = t->internal.write.webService.send;

	httpService( &(t->internal.write.webService) );

	t->internal.write.webService.send = 0;
	t->internal.write.webService.abort = 0;


} // End Fn
