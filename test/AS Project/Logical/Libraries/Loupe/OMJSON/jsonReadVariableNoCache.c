/*
 * File: jsonReadVariableNoCache.c
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
// Convert a variable into a JSON object without caching			*
//*******************************************************************

// Passes first tests with complex structure

void jsonReadVariableNoCache(struct jsonReadVariableNoCache* t)
{

	//***********************************************
	// Check for invalid inputs						*
	//***********************************************
	
	if( t == 0 ) return;
	if( t->pVariableName == 0 ){ jsonInternalSetReadNoCacheError(JSON_ERR_INVALIDINPUT, t); return; }

	
	//***************************************************
	// Initialize 										*
	//***************************************************

	if( !t->internal.initialized ){
		
		// Clear internals
		memset( &(t->internal), 0, sizeof(t->internal) );
		
		// Set defaults
		if( t->BufferSize == 0 ) t->BufferSize = JSON_DEFAULT_BUFFERSIZE;
		if( t->MaxIterations == 0 ) t->MaxIterations = JSON_DEFAULT_MAXITERATIONS;
		
		// Initialize buffer
		if(	datbufInitBuffer( (UDINT)&(t->internal.outputBuffer), t->BufferSize ) != 0 ){
			jsonInternalSetReadNoCacheError(JSON_ERR_MEMALLOC, t);
			return;
		}
		
		t->internal.initialized = 1;
		
	}// if(!initialized)

	
	//***********************************************
	// Convert variable to JSON						*
	//***********************************************
	
	UDINT	i, iArray;
	UINT	clearStatus, appendStatus, infoStatus, itemStatus, valueStatus;
	STRING	elementName[VAR_STRLEN_NAME + 1], 
			iArrayString[10 + 1];
	varVariable_typ	arrayElement;
	char	*pChar;
	
	
	// If you are at the top level,	copy VariableName to structData
	// clear buffer, start up buffer
	// This will not work if you are reading a list of variables
	
	if( t->internal.iStructLevel == 0 ){
	
		// Initialize variable name
		strcpy( t->internal.structLevel[0].variable.name, "" );
		pChar = (char*)t->pVariableName;
		pChar = skip(pChar);
		if( *pChar != '\"' ){ t->Status = JSON_ERR_PARSE; return; }
		pChar = parse_string((char*)t->internal.structLevel[0].variable.name, pChar, sizeof(t->internal.structLevel[0].variable.name) - 1);
		if( pChar == 0 ){
			t->Status = JSON_ERR_PARSE;
			return;
		}
		
		// Clear buffer
		clearStatus = datbufClearBuffer( (UDINT)&(t->internal.outputBuffer) );
		
		if( clearStatus != 0 ){
			jsonInternalSetReadNoCacheError(clearStatus, t);
			return;
		}
		
		// Open up object with variable name '{"variableName":'
		appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("{"), 1 );
		appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), 
												t->pVariableName, 
												strlen((char*)t->pVariableName) );
		appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&(":"), 1 );
		
		if( appendStatus != 0 ){
			jsonInternalSetReadNoCacheError(appendStatus, t);
			return;
		}
			
	} // if(iStructLevel == 0)
	
	
	// Traverse structure looking for simple variables
	
	for( i = 0; i < t->MaxIterations; i++ ){
		
		
		// If necessary, get var info (new variable)
		if( t->internal.structLevel[t->internal.iStructLevel].variable.length  == 0 ){
		
			// Get var info for current level and reset current item index
			infoStatus=	varGetInfo( (UDINT)&(t->internal.structLevel[t->internal.iStructLevel].variable) );
			
			if( infoStatus != 0 ){
				jsonInternalSetReadNoCacheError(infoStatus, t);
				return;
			}
			
			t->internal.structLevel[t->internal.iStructLevel].iItem=	0;
					
		} // If current level is empty
				
		
		// switch off datatype - 
		//	If it is a structure then you need to get current structure element and move down one level
		//	If it is an array of structures, then you need to get current array element and move down one level
		//	If it is a basic data point, then you need to insert the variable values and move up to the proper level and item
		
		switch( t->internal.structLevel[t->internal.iStructLevel].variable.dataType ){
			
			
			//***************************************************
			// Single structure		 							*
			//***************************************************
			
			case VAR_TYPE_STRUCT:	
				
				// Check current level
				if( t->internal.iStructLevel == JSON_MAI_STRUCTLEVEL){
					jsonInternalSetReadNoCacheError(JSON_ERR_MAXLEVELREACHED, t);
					return;
				}
				
				// Get Current Structure Element name
				itemStatus=	PV_item( 	t->internal.structLevel[t->internal.iStructLevel].variable.name,
										t->internal.structLevel[t->internal.iStructLevel].iItem,
										elementName );
				
				if( itemStatus != 0 ){
			
					jsonInternalSetReadNoCacheError(itemStatus, t);
					return;
					
					// DFB 20130627 - This is where having a dummy structure as element breaks the expansion.
					//	If you want to recover from this, you will have to skip this item appropriately.
					//	Not sure how to do that right now.
					// DFB 20140924 - Huh?
			
				} // Error with PV_Item
				
				// If looking at first item in structure, need to open structure
				// by appending "{"
				if( t->internal.structLevel[t->internal.iStructLevel].iItem == 0 ){
					appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("{"), 1 );
				}
				
				// Append ""elementName":"
				appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("\""), 1 );
				appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), 
														(UDINT)&(elementName), 
														strlen(elementName) );
				appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("\":"), 2 );
				
				if( appendStatus != 0 ){
					jsonInternalSetReadNoCacheError(appendStatus, t);
					return;
				}
				
				// Set next level name to current structure element,
				// go down one level
				strcpy( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, "" );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, t->internal.structLevel[t->internal.iStructLevel].variable.name,	VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, ".", 																VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, elementName, 														VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				
				t->internal.iStructLevel++;
				
				break;
				
			
			//***************************************************
			// Array of structures 								*
			//***************************************************
			
			case VAR_TYPE_ARRAY_OF_STRUCT:
				
				// Check current level
				if( t->internal.iStructLevel == JSON_MAI_STRUCTLEVEL){
					jsonInternalSetReadNoCacheError(JSON_ERR_MAXLEVELREACHED, t);
					return;
				}
				
				// If looking at first item in array, need to open array
				// by appending "["
				if( t->internal.structLevel[t->internal.iStructLevel].iItem == 0 ){
					appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("["), 1 );
				}
				
				if( appendStatus != 0 ){
					jsonInternalSetReadNoCacheError(appendStatus, t);
					return;
				}
				
				// Set next level name to current array element,
				// go down one level
				strcpy( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, "" );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, t->internal.structLevel[t->internal.iStructLevel].variable.name, VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, "[", VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				brsitoa( t->internal.structLevel[t->internal.iStructLevel].iItem, (UDINT)iArrayString );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, iArrayString, VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				strncat( t->internal.structLevel[t->internal.iStructLevel + 1].variable.name, "]", VAR_STRLEN_NAME - strlen(t->internal.structLevel[t->internal.iStructLevel + 1].variable.name) );
				
				t->internal.iStructLevel++;
				
				break;
				
				
			//***************************************************
			// Basic variable									*
			//***************************************************
		
			default:
				
				// Single basic variable
				if( t->internal.structLevel[t->internal.iStructLevel].variable.dimension == 1 ){
					
					// Get value
					valueStatus = varGetValue( (UDINT)&(t->internal.structLevel[t->internal.iStructLevel].variable) );
					
					if( valueStatus != 0 ){
						jsonInternalSetReadNoCacheError(valueStatus, t);
						return;
					}
					
					
					if( t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_STRING ){
						// Stringify - Do not use variable.value. It is limited to 120 characters, which is not enough
						// for strings.
						// TODO: This can cause a stack overflow if variable.length is very large.
						//		Maybe add a max size
						STRING tempString[t->internal.structLevel[t->internal.iStructLevel].variable.length];
						char* reentry;
						stringify_string(tempString, (char*)t->internal.structLevel[t->internal.iStructLevel].variable.address, sizeof(tempString) - 1, &reentry);
			
						// Append ""tempString""
						appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("\""), 1 );
						appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&tempString, strlen(tempString) );
						if(reentry != 0) {
							stringify_string(tempString, reentry, sizeof(tempString) - 1, &reentry);
							appendStatus = datbufAppendToBuffer((UDINT)&(t->internal.outputBuffer), (UDINT)&tempString, strlen(tempString));
						}
						appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("\""), 1 );
					
					}
					else{
						// Append "value"
						appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), 
																(UDINT)&(t->internal.structLevel[t->internal.iStructLevel].variable.value), 
																strlen(t->internal.structLevel[t->internal.iStructLevel].variable.value) );
					}
					
					if( appendStatus != 0 ){
						jsonInternalSetReadNoCacheError(appendStatus, t);
						return;
					}
					
				} // Single variable
				
				// Array of basic variables
				else{
					
					// Append "["
					appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("["), 1 );
					
					if( appendStatus != 0 ){
						jsonInternalSetReadNoCacheError(appendStatus, t);
						return;
					}
					
					// Loop through array
					for( iArray=0; iArray < t->internal.structLevel[t->internal.iStructLevel].variable.dimension; iArray++ ){
						
						// Get variable name for each array element
						strcpy( arrayElement.name, "" );
						strncat( arrayElement.name, t->internal.structLevel[t->internal.iStructLevel].variable.name, VAR_STRLEN_NAME );
						strncat( arrayElement.name, "[", VAR_STRLEN_NAME - strlen(arrayElement.name) );
						brsitoa( iArray, (UDINT)iArrayString );
						strncat( arrayElement.name, iArrayString, VAR_STRLEN_NAME - strlen(arrayElement.name) );
						strncat( arrayElement.name, "]", VAR_STRLEN_NAME - strlen(arrayElement.name) );
					
						// Get info and value
						infoStatus = varGetInfo( (UDINT)&(arrayElement) );
						
						if( infoStatus != 0 ){
							jsonInternalSetReadNoCacheError(infoStatus, t);
							return;
						}
						
						valueStatus = varGetValue( (UDINT)&(arrayElement) );
						
						if( valueStatus != 0 ){
							jsonInternalSetReadNoCacheError(valueStatus, t);
							return;
						}
						
						if( t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_STRING ){
							// Stringify - Do not use variable.value. It is limited to 120 characters, which is not enough
							// for strings.
							// TODO: This can cause a stack overflow if variable.length is very large.
							//		Maybe add a max size
							STRING tempString[arrayElement.length];
							char* reentry;
							stringify_string(tempString, (char*)arrayElement.address, sizeof(tempString) - 1, &reentry);
			
							// Append ""tempString""
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("\""), 1 );
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&tempString, strlen(tempString) );
							if(reentry != 0) {
								stringify_string(tempString, reentry, sizeof(tempString) - 1, &reentry);
								appendStatus = datbufAppendToBuffer((UDINT)&(t->internal.outputBuffer), (UDINT)&tempString, strlen(tempString));
							}
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("\""), 1 );
							
						}
						else{
							// Append "value"
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), 
																	(UDINT)&(arrayElement.value), 
																	strlen(arrayElement.value) );
						}
					
						// If last item, append "]", else, append ","
						if( iArray == (t->internal.structLevel[t->internal.iStructLevel].variable.dimension - 1) ){
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("]"), 1 );
						}
						else{
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&(","), 1 );
						}
						
						if( appendStatus != 0 ){
							jsonInternalSetReadNoCacheError(appendStatus, t);
							return;
						}
						
					} // loop through array elements
					
				} // Array of variables
				
				// Clear current level
				memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[t->internal.iStructLevel]) );
				
				// Find next item (iLevel, iItem)
				while(1){
					
					// Move up one level
					t->internal.iStructLevel--;
					
					// If you are above level 0, all done
					if( t->internal.iStructLevel < 0 ){
						
						t->internal.iStructLevel = 0;
						
						// Close object with an extra '}'
						appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("}"), 1 );
						
						if( appendStatus != 0 ){
							jsonInternalSetReadNoCacheError(appendStatus, t);
							return;
						}
						
						t->Status = 0;
						t->pJSONObject = t->internal.outputBuffer.pData;
						t->JSONObjectLength = t->internal.outputBuffer.currentLength;
						return;
						
					} // Done
					
					
					// If the level up exists, then it is a structure or array of structures
					// Maybe check dataType, anyway?
					// Probably need to have different behavior for structures and arrays of structures
					
					// Increment iItem and check against dimension
					t->internal.structLevel[t->internal.iStructLevel].iItem++;
		
					if( t->internal.structLevel[t->internal.iStructLevel].iItem < t->internal.structLevel[t->internal.iStructLevel].variable.dimension ){
				
						// Need to do more with this structure or array, 
						// append "," and keep going
						appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&(","), 1 );
					
						if( appendStatus != 0 ){
							jsonInternalSetReadNoCacheError(appendStatus, t);
							return;
						}
						break;
				
					} // keep going with this structure
			
			
					else{
				
						// Done with this structure or array, 
						// Close the structure (append "}") or array (append "]")
						if( t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_STRUCT ){
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("}"), 1 );
						}
						else if( t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_ARRAY_OF_STRUCT ){
							appendStatus = datbufAppendToBuffer( (UDINT)&(t->internal.outputBuffer), (UDINT)&("]"), 1 );
						}
						else{
							jsonInternalSetReadNoCacheError(JSON_ERR_HUH, t);
							return;
						}
					
						if( appendStatus != 0 ){
							jsonInternalSetReadNoCacheError(appendStatus, t);
							return;
						}
						
						// Clear this level and move up (stay in loop)
						memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[t->internal.iStructLevel]) );
							
					} // Move up one more level
					
					
				} // Loop to go up levels
					
				break;
			
		} // switch(DataType)
	
	} // Traverse loop
	
	// Fix reentry later
	jsonInternalSetReadNoCacheError(JSON_ERR_MAXITERATIONS, t);
	return;

} // End Fn
