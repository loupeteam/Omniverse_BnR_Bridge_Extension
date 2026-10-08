/*
 * File: jsonWriteVariable.c
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

#ifdef __cplusplus
	};
#endif


//*******************************************************************
// Write variable values received as a JSON object 					*
//*******************************************************************

// Passes first tests with complex structure

void jsonWriteVariable(struct jsonWriteVariable* t)
{

	//***********************************************
	// Check for invalid inputs						*
	//***********************************************
	
	if( t == 0 ) return;
	if( t->pJSONObject == 0 ){ t->Status = JSON_ERR_INVALIDINPUT; return; }

	
	//***************************************************
	// Initialize 										*
	//***************************************************

	if( !t->internal.initialized ){
		
		// Clear internals
		memset( &(t->internal), 0, sizeof(t->internal) );
		
		// Set defaults
		if( t->MaxJSONObjectLength == 0 ) t->MaxJSONObjectLength = JSON_DEFAULT_MAXJSONOBJECTLENGTH;
		if( t->MaxIterations == 0 ) t->MaxIterations = JSON_DEFAULT_MAXITERATIONS;
		
		// Allocate memory
		if(	TMP_alloc( t->MaxJSONObjectLength+1, (void**)&(t->internal.pJSONObject) ) != 0 ){
			t->Status = JSON_ERR_MEMALLOC;
			return;
		}
		
		memset( (void*)t->internal.pJSONObject, 0, t->MaxJSONObjectLength+1 );
		
		t->internal.iStructLevel = -1; // Since there is a wrapper JSON object in the interface, -1 is a valid structure level
		
		t->internal.initialized = 1;
		
	}// if(!initialized)

	
	//***********************************************
	// Write JSON object							*
	//***********************************************
	
	// Use these internally - store them to t->internal at the end for reentry
	char *pJSONObject, *pChar;
	pJSONObject = (char*)t->internal.pJSONObject;
	pChar = (char*)t->internal.pChar;
	
	STRING propName[VAR_STRLEN_NAME+1];
	strcpy(propName, "");
	
	STRING iArrayString[10+1];
	strcpy(iArrayString,"");
	
	// First time things - fix reentrance later
//	if( t->Status != ERR_FUB_BUSY ){
	
		// Get internal copy of JSON object - do we need to???
		memset( (void*)pJSONObject, 0, t->MaxJSONObjectLength+1 );
		strncpy( pJSONObject, (char*)t->pJSONObject, t->MaxJSONObjectLength );
		
		// Initialize parsing things
		pChar = pJSONObject;
		t->internal.iStructLevel = -1;
		
//	} // if(not busy)

	UDINT i;
	
	for( i = 0; i < t->MaxIterations; i++ ){
	
		// Want to parse one property:value pair per iteration.
		
		// Get property name
		// Check for object/array openers '{' '['
		pChar = skip(pChar);
		switch( *pChar ){
	
			case '{':
				
				// New object. Need to go down one level and grab property name
				t->internal.iStructLevel++;
				
				if( t->internal.iStructLevel > JSON_MAI_STRUCTLEVEL ){
					t->Status = JSON_ERR_MAXLEVELREACHED;
					return;
				}
				
				memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[0]) );
				
				if( t->internal.iStructLevel > 0 ){
					// Start off with previous level variable name and add '.'
					strncpy( t->internal.structLevel[t->internal.iStructLevel].variable.name, t->internal.structLevel[t->internal.iStructLevel - 1].variable.name, VAR_STRLEN_NAME - 1 );
					strcat( t->internal.structLevel[t->internal.iStructLevel].variable.name, "." );
				}
				
				// Copy current property name into variable name
				pChar++;
				pChar = skip(pChar);
				if( *pChar != '\"' ){ t->Status = JSON_ERR_PARSE; return; }
				pChar = parse_string(propName, pChar, sizeof(propName) - 1);
				if( pChar == 0 ){
					t->Status= JSON_ERR_PARSE;
					return;
				}
				strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, propName, VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
				
				t->internal.structLevel[t->internal.iStructLevel].jsonType = JSON_TYPE_OBJECT_ELEMENT;
				
				break;
				
			
			case '[':
				
				// New array
				if( t->internal.iStructLevel < 0 ){ t->Status = JSON_ERR_PARSE; return; }
				
				// Current level will be array 'name' and will handle array index
				// Variable.name should already be populated above as an object element
				t->internal.structLevel[t->internal.iStructLevel].iItem = 0;
				
				// Use next level as array element
				t->internal.iStructLevel++;
				
				if( t->internal.iStructLevel > JSON_MAI_STRUCTLEVEL ){
					t->Status = JSON_ERR_MAXLEVELREACHED;
					return;
				}
				
				memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[0]) );
				
				// Start off with previous level variable name and add array index
				strncpy( t->internal.structLevel[t->internal.iStructLevel].variable.name, t->internal.structLevel[t->internal.iStructLevel - 1].variable.name, VAR_STRLEN_NAME );
				strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, "[", VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
				brsitoa( t->internal.structLevel[t->internal.iStructLevel - 1].iItem, (UDINT)iArrayString );
				strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, iArrayString, VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
				strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, "]", VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
				
				t->internal.structLevel[t->internal.iStructLevel].jsonType = JSON_TYPE_ARRAY_ELEMENT;
				
				pChar++;
				
				break;
				
			case ',': 
				
				if( t->internal.structLevel[t->internal.iStructLevel].jsonType == JSON_TYPE_OBJECT_ELEMENT ){

					// New property of current object
					memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[0]) );
					
					if( t->internal.iStructLevel > 0 ){
						// Start off with previous level variable name and add '.'
						strncpy( t->internal.structLevel[t->internal.iStructLevel].variable.name, t->internal.structLevel[t->internal.iStructLevel - 1].variable.name, VAR_STRLEN_NAME - 1 );
						strcat( t->internal.structLevel[t->internal.iStructLevel].variable.name, "." );
					}
					
					// Copy current property name into variable name
					pChar++;
					pChar = skip(pChar);
					if( *pChar != '\"' ){ t->Status = JSON_ERR_PARSE; return; }
					pChar = parse_string(propName, pChar, sizeof(propName) - 1);
					if( pChar == 0 ){
						t->Status = JSON_ERR_PARSE;
						return;
					}
					strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, propName, VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
					
					t->internal.structLevel[t->internal.iStructLevel].jsonType = JSON_TYPE_OBJECT_ELEMENT;
				
				}
				else if( t->internal.structLevel[t->internal.iStructLevel].jsonType == JSON_TYPE_ARRAY_ELEMENT ){
				
					// Next array element
					if( t->internal.iStructLevel <= 0 ){ t->Status = JSON_ERR_PARSE; return; }
				
					t->internal.structLevel[t->internal.iStructLevel - 1].iItem++;
					
					memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[0]) );
					
					// Start off with previous level variable name and add array index
					strncpy( t->internal.structLevel[t->internal.iStructLevel].variable.name, t->internal.structLevel[t->internal.iStructLevel - 1].variable.name, VAR_STRLEN_NAME );
					strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, "[", VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
					brsitoa( t->internal.structLevel[t->internal.iStructLevel - 1].iItem, (UDINT)iArrayString );
					strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, iArrayString, VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
					strncat( t->internal.structLevel[t->internal.iStructLevel].variable.name, "]", VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel].variable.name) );
					
					t->internal.structLevel[t->internal.iStructLevel].jsonType = JSON_TYPE_ARRAY_ELEMENT;
					
					pChar++;
				
				}
				else{
					t->Status = JSON_ERR_PARSE;
					return;
				}
		
				break;
				
		} // switch(*pChar)
		
		// If you are in an object element, should have :
		// Otherwise, don't have :
		pChar = skip(pChar);
		if( t->internal.structLevel[t->internal.iStructLevel].jsonType == JSON_TYPE_OBJECT_ELEMENT ){
			if( *pChar != ':'){ t->Status = JSON_ERR_PARSE; return; }
			pChar++;
		}
		
		// Get value
		pChar = skip(pChar);
		pChar = parse_value( &(t->internal.structLevel[t->internal.iStructLevel].variable), pChar );
		if( pChar == 0 ){
			t->Status = JSON_ERR_PARSE;
			return;
		}
		
		// Check for object/array closers '}' ']'
		pChar = skip(pChar);
		while( *pChar && (*pChar=='}' || *pChar==']') ){
			
			/*switch( *pChar ){
			
				case '}':
				case ']':
					
					// Close an object.
					// Move up a structure level.
					if( t->internal.iStructLevel >= 0 ){
						memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[0]) );
						t->internal.iStructLevel--;
					}
					else{
						t->Status = JSON_ERR_PARSE; 
						return; 
					}
					
					break;
					
			} // switch(*pChar) */
			
			// Close an object or array
			// Move up a structure level
			if( t->internal.iStructLevel >= 0 ){
				memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[0]) );
				t->internal.iStructLevel--;
			}
			else{
				t->Status = JSON_ERR_PARSE; 
				return; 
			}
			
			pChar++;
			pChar = skip(pChar);
		
		} // Loop through all closers
		
		// Done???
		if( *pChar == 0 ){ t->Status = 0; return; }
		else if( *pChar == '{' ){}// Move on (do nothing)
		else if( *pChar == '[' ){}// Move on (do nothing)
		else if( *pChar == ',' ){}// Move on (do nothing)
		else{ t->Status = JSON_ERR_PARSE; return; }
	
	} // Loop

	// Prepare for reentry - fix this later
	t->internal.pChar = (UDINT)pChar;
	t->Status = JSON_ERR_MAXITERATIONS;
	return;
			
} // End Fn
