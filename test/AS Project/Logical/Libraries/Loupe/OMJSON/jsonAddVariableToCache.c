/*
 * File: jsonAddVariableToCache.c
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
// Add a variable to the JSON variable cache 						*
//*******************************************************************


void jsonAddVariableToCache(struct jsonAddVariableToCache* t)
{

	//***********************************************
	// Check for invalid inputs						*
	//***********************************************
	
	if( t == 0 ) return;
	
	if( (t->pVariableName == 0) || (t->pCache == 0) ){
		jsonInternalSetAddError(JSON_ERR_INVALIDINPUT, t);
		return;
	}
	
	//***************************************************
	// Initialize 										*
	//***************************************************

	if( !t->internal.initialized ){
		
		// Clear internals
		memset( &(t->internal), 0, sizeof(t->internal) );
		
		// Set defaults
		if( t->MaxIterations == 0 ) t->MaxIterations = JSON_DEFAULT_MAXITERATIONS;
		
		t->internal.initialized = 1;
		
	}// if(!initialized)

	
	//***********************************************
	// Add variable to cache						*
	//***********************************************
	
	UDINT	i, iArray;
	UINT	infoStatus, itemStatus;
	STRING	elementName[VAR_STRLEN_NAME + 1], 
	iArrayString[10 + 1];
	varVariable_typ	arrayElement;
	jsonCache_typ	*pCache;
	
	// Grab pCache
	pCache = (jsonCache_typ*)(t->pCache);
	
	// If you are at the top level,	copy VariableName to structData
	// Need to distinguish first scan from subsequent scans
	// For now, not a reentrant function, so no need to do this
	//	if( t->internal.iStructLevel == 0 ){

	// Check iItem and iVariable
	if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLITEMS, t); return; }
	if( pCache->iVariable > JSON_MAI_CACHEVAR ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLVARS, t); return; }
	
	// Add variable to list of variables in cache
	strcpy( pCache->variable[pCache->iVariable].name, (char*)t->pVariableName );
	pCache->variable[pCache->iVariable].iStart = pCache->iItem;
	
	// Initialize expansion structure
	strcpy( t->internal.structLevel[0].variable.name, (char*)t->pVariableName );
		
	// Start prefix with variable name '{"variableName":'
	strcpy( pCache->item[pCache->iItem].prefix, "" );
//	strcat( pCache->item[pCache->iItem].prefix, "{\"" );
	strcat( pCache->item[pCache->iItem].prefix, "\"" );
	strcat( pCache->item[pCache->iItem].prefix, (char*)t->pVariableName );
	strcat( pCache->item[pCache->iItem].prefix, "\":" );
		
	//	} // if(iStructLevel == 0)
	
	// Traverse structure looking for simple variables
	for( i = 0; i < t->MaxIterations; i++ ){
		
		
		// If necessary, get var info (new variable)
		if( t->internal.structLevel[t->internal.iStructLevel].variable.length == 0 ){
		
			// Get var info for current level and reset current item index
			infoStatus = varGetInfo( (UDINT)&(t->internal.structLevel[t->internal.iStructLevel].variable) );
			
//			if( infoStatus != 0 ){
//				jsonInternalSetAddError(infoStatus, t);
//				return;				
//			}
			
			t->internal.structLevel[t->internal.iStructLevel].iItem=	0;
					
		} // If current level is empty
				
		
		// switch off datatype - 
		//	If it is a structure then you need to get current structure element and move down one level
		//	If it is an array of structures, then you need to get current array element and move down one level
		//	If it is a basic data point, then you need to insert the variable info and move up to the proper level and item
		
		switch( t->internal.structLevel[t->internal.iStructLevel].variable.dataType ){
			
			
			//***************************************************
			// Single structure		 							*
			//***************************************************
			
			case VAR_TYPE_STRUCT:	
				
				// Check current level
				if( t->internal.iStructLevel == JSON_MAI_STRUCTLEVEL){ jsonInternalSetAddError(JSON_ERR_MAXLEVELREACHED, t); return; }
				
				// Get Current Structure Element name
				itemStatus = PV_item( 	t->internal.structLevel[t->internal.iStructLevel].variable.name,
				t->internal.structLevel[t->internal.iStructLevel].iItem,
				elementName );
				
				if( itemStatus != 0 ){ jsonInternalSetAddError(itemStatus, t); return; }
				
				// If looking at first item in structure, need to open structure by appending "{"
				if( t->internal.structLevel[t->internal.iStructLevel].iItem == 0 ){
					if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLITEMS, t); return; }
					strcat( pCache->item[pCache->iItem].prefix, "{" );
				}
				
				// Append ""elementName":"
				if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLITEMS, t); return; }
				strcat( pCache->item[pCache->iItem].prefix, "\"" );
				strcat( pCache->item[pCache->iItem].prefix, elementName );
				strcat( pCache->item[pCache->iItem].prefix, "\":" );
				
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
				if( t->internal.iStructLevel == JSON_MAI_STRUCTLEVEL ){ jsonInternalSetAddError(JSON_ERR_MAXLEVELREACHED, t); return; }
				
				// If looking at first item in array, need to open array by appending "["
				if( t->internal.structLevel[t->internal.iStructLevel].iItem == 0 ){
					if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLITEMS, t); return; }
					strcat( pCache->item[pCache->iItem].prefix, "[" );
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
			// Undefined										*
			//***************************************************
		
//			case VAR_TYPE_UNDEFINED:
//				
//				// Add variable to cache
//				if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULL, t); return; }
//					
//				memcpy( &(pCache->item[pCache->iItem].variable), 
//					&(t->internal.structLevel[t->internal.iStructLevel].variable), 
//					sizeof(pCache->item[pCache->iItem].variable) );
//					
//				strcpy( pCache->item[pCache->iItem].suffix, "" );
//					
//				pCache->iItem++;
//					
//				if( pCache->iItem <= JSON_MAI_CACHEITEM ) strcpy( pCache->item[pCache->iItem].prefix, "" );
//				
//				break;
			
			
			//***************************************************
			// Basic variable or undefined						*
			//***************************************************
		
			default:
				
				// Single basic variable or variable not found
				if( 	(t->internal.structLevel[t->internal.iStructLevel].variable.dimension == 1)
					||	(t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_UNDEFINED)
				){
					
					// Add variable to cache
					if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLITEMS, t); return; }
					
					memcpy( &(pCache->item[pCache->iItem].variable), 
						&(t->internal.structLevel[t->internal.iStructLevel].variable), 
						sizeof(pCache->item[pCache->iItem].variable) );
					
					strcpy( pCache->item[pCache->iItem].suffix, "" );
					
					pCache->iItem++;
					
					if( pCache->iItem <= JSON_MAI_CACHEITEM ) strcpy( pCache->item[pCache->iItem].prefix, "" );

				} // Single variable
				
				
				// Array of basic variables
				else{
					
					// Append "[" to prefix
					strcat( pCache->item[pCache->iItem].prefix, "[" );
				
					// Loop through array
					for( iArray = 0; iArray < t->internal.structLevel[t->internal.iStructLevel].variable.dimension; iArray++ ){
						
						// Get variable name for each array element
						strcpy( arrayElement.name, "" );
						strncat( arrayElement.name, t->internal.structLevel[t->internal.iStructLevel].variable.name, VAR_STRLEN_NAME );
						strncat( arrayElement.name, "[", VAR_STRLEN_NAME - strlen(arrayElement.name) );
						brsitoa( iArray, (UDINT)iArrayString );
						strncat( arrayElement.name, iArrayString, VAR_STRLEN_NAME - strlen(arrayElement.name) );
						strncat( arrayElement.name, "]", VAR_STRLEN_NAME - strlen(arrayElement.name) );
					
						// Get variable info
						infoStatus = varGetInfo( (UDINT)&(arrayElement) );
						if( infoStatus != 0 ){ jsonInternalSetAddError(infoStatus, t); return; }
						
						// Add variable to cache
						if( pCache->iItem > JSON_MAI_CACHEITEM ){ jsonInternalSetAddError(JSON_ERR_CACHEFULLITEMS, t); return; }
						
						memcpy( &(pCache->item[pCache->iItem].variable), 
							&(arrayElement), 
							sizeof(pCache->item[pCache->iItem].variable) );
						
						if( iArray == t->internal.structLevel[t->internal.iStructLevel].variable.dimension - 1 ){
							strcpy( pCache->item[pCache->iItem].suffix, "]" );
						}
						else{
							strcpy( pCache->item[pCache->iItem].suffix, "," );
						}
						
						pCache->iItem++;
						
						if( pCache->iItem <= JSON_MAI_CACHEITEM ) strcpy( pCache->item[pCache->iItem].prefix, "" );
						
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
						
						// Close object with an one last '}'
						//if( pCache->iItem == 0 ){ jsonInternalSetAddError(JSON_ERR_CACHEEMPTY, t); return; }
						//strcat( pCache->item[pCache->iItem-1].suffix, "}" );
						
						// Update variable info in cache
						pCache->variable[pCache->iVariable].iEnd = pCache->iItem - 1;
						pCache->iVariable++;
				
						t->Status = 0;
						t->iVariable = pCache->iVariable - 1;
				
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
						if( pCache->iItem == 0 ){ jsonInternalSetAddError(JSON_ERR_CACHEEMPTY, t); return; }
						strcat( pCache->item[pCache->iItem-1].suffix, "," );
						
						break;
				
					} // keep going with this structure
			
					else{
				
						// Done with this structure or array, 
						// Close the structure (append "}") or array (append "]")
						if( t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_STRUCT ){
							if( pCache->iItem == 0 ){ jsonInternalSetAddError(JSON_ERR_CACHEEMPTY, t); return; }
							strcat( pCache->item[pCache->iItem-1].suffix, "}" );
						}
						else if( t->internal.structLevel[t->internal.iStructLevel].variable.dataType == VAR_TYPE_ARRAY_OF_STRUCT ){
							if( pCache->iItem == 0 ){ jsonInternalSetAddError(JSON_ERR_CACHEEMPTY, t); return; }
							strcat( pCache->item[pCache->iItem-1].suffix, "]" );
						}
						else{
							jsonInternalSetAddError(JSON_ERR_HUH, t);
							return;
						}
					
						// Clear this level and move up (stay in loop)
						memset( &(t->internal.structLevel[t->internal.iStructLevel]), 0, sizeof(t->internal.structLevel[t->internal.iStructLevel]) );
							
					} // Move up one more level
					
					
				} // Loop to go up levels
					
				break;
			
		} // switch(DataType)

	} // Traverse loop
	
	// For now, crap out. Fix reentrance later.
	jsonInternalSetAddError(JSON_ERR_MAXITERATIONS, t);
	return;
	
} // End Fn
