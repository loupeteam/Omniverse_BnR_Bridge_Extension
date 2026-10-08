/*
 * File: jsonReadVariableList.c
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
// Convert a list of variables into a JSON object					*
//*******************************************************************

void jsonReadVariableList(struct jsonReadVariableList* t)
{

	// Check for invalid inputs
	if( t == 0 ) return;
	
	if( (t->pVariableList == 0) || (t->pCache == 0) ){
		jsonInternalSetReadListError(JSON_ERR_INVALIDINPUT, t);
		return;
	}
	
	// Initialize
	if( !t->internal.initialized ){
		
		// Clear internals
		memset( &(t->internal), 0, sizeof(t->internal) );
		
		// Set defaults
		if( t->BufferSize == 0 ) t->BufferSize = JSON_DEFAULT_BUFFERSIZE;
		if( t->MaxIterations == 0 ) t->MaxIterations = JSON_DEFAULT_MAXITERATIONS;
		
		// Initialize buffer
		if(	datbufInitBuffer( (UDINT)&(t->internal.outputBuffer), t->BufferSize ) != 0 ){
			jsonInternalSetReadListError(JSON_ERR_MEMALLOC, t);
			return;
		}
		
		t->internal.initialized = 1;
		
	} // if(!initialized)

	// Check for empty var list
	if (strcmp((char*)t->pVariableList, "") == 0) {
		t->Status = 0;
		t->pJSONObject = (UDINT)&("[]");
		t->JSONObjectLength = 2;
		return;
	}
	
	// Clear buffer
	UINT clearStatus = datbufClearBuffer( (UDINT)&(t->internal.outputBuffer) );
	if( clearStatus != 0 ){ jsonInternalSetReadListError(clearStatus, t); return; }
	
	// Parse pVariableList
	char* pChar = (char*)t->pVariableList;
	pChar = skip(pChar);
	
	// If pVariableList starts with a [, skip it and add a "[" to the output object
	UINT appendStatus;
	
	if( *pChar == '[' ){
	
		appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("["), 1 );
		if( appendStatus != 0 ){ jsonInternalSetReadListError(appendStatus, t); return; }
		pChar++;
	
		// Check for empty array
		pChar = skip(pChar);
		if( *pChar == ']' ){
			appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("]"), 1 );
			if( appendStatus != 0 ){ jsonInternalSetReadListError(appendStatus, t); return; }
			// All done. Set outputs and return.
			t->Status = 0;
			t->pJSONObject = t->internal.outputBuffer.pData;
			t->JSONObjectLength = t->internal.outputBuffer.currentLength;
			return;
		}
		
	} // Opening '['
	
	// Loop through, processing one varName at a time
	STRING varName[VAR_STRLEN_NAME + 1];
	UDINT i;
	
	for( i = 0; i < t->MaxIterations; i++ ){
		
		memset( &varName, 0, sizeof(varName) );
	
		// Only allow varNames with quotes right now
		// This supports the output of JSON.stringify(stringArray)
		pChar = skip(pChar);
		if (*pChar != '\"') {jsonInternalSetReadListError(JSON_ERR_PARSE, t); return;}
		pChar = parse_string(varName, pChar, sizeof(varName) - 1);
		if (pChar == 0) {
			jsonInternalSetReadListError(JSON_ERR_PARSE, t);
			return;
		}
		
		// Read varName
		t->internal.readVariable.pVariableName = (UDINT)&varName;
		t->internal.readVariable.pCache = t->pCache;
		t->internal.readVariable.BufferSize = t->BufferSize;
		t->internal.readVariable.MaxIterations = t->MaxIterations;
			
		jsonReadVariable(&t->internal.readVariable);
		
		if( t->internal.readVariable.Status != 0 ){ jsonInternalSetReadListError(t->internal.readVariable.Status, t); return; }
		
		// Append ReadVariable output to buffer
		appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), 
			t->internal.readVariable.pJSONObject,
			t->internal.readVariable.JSONObjectLength );
		if( appendStatus != 0 ){ jsonInternalSetReadListError(appendStatus, t); return; }
		
		// Check what is after the varName
		pChar = skip(pChar);
		if( *pChar == ',' ){
			// Append ","
			appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&(","), 1 );
			if( appendStatus != 0 ){ jsonInternalSetReadListError(appendStatus, t); return; }
			pChar++;
		}
		else if( *pChar == ']' ){
			// Append "]"
			appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("]"), 1 );
			if( appendStatus != 0 ){ jsonInternalSetReadListError(appendStatus, t); return; }
			pChar++;
		}

		// Check for end of pVariableList
		pChar = skip(pChar);
		if( *pChar == 0 ){
			// All done. Set outputs and return.
			t->Status = 0;
			t->pJSONObject = t->internal.outputBuffer.pData;
			t->JSONObjectLength = t->internal.outputBuffer.currentLength;
			return;
		}
		
	} // Loop through pVariableList
	
	// No reentrance right now
	jsonInternalSetReadListError(JSON_ERR_MAXITERATIONS, t);
	return;

} // End Fn
