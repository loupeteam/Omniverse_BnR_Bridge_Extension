/*
 * File: varPopulateMemberNames.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of VarTools, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

	#include "varToolsInternal.h"
	#include "VarTools.h"

#ifdef __cplusplus
	};
#endif

#include <string.h>
#include <stdlib.h>

plcbit varPopulateMemberNames( plcstring * VariableName, unsigned char prefix )
{	
	varVariable_typ v; 
	strncpy( v.name, VariableName, sizeof(v.name));
	varGetInfo( (UDINT) &v );

	STRING variableName[200];
	STRING memberName[38];
	STRING arrayIndex[4];
	int memberIndex = 0;

	strcpy( variableName, VariableName);

	switch ( v.dataType )
	{
	case VAR_TYPE_STRING:
			if( v.dimension > 1 ){
				for( memberIndex = 0; memberIndex < v.dimension; memberIndex++ ){
					strcpy( variableName, VariableName);
					strncat( variableName,"[", sizeof(variableName) );
					itoa(memberIndex, arrayIndex, 10);	
					strncat( variableName, arrayIndex, sizeof(variableName));
					strncat( variableName,"]", sizeof(variableName) );
					varPopulateMemberNames( variableName, prefix );			
				}
			}
			else{
				int len = strlen(variableName);
				if( variableName[ len - 2] == '.' && variableName[ len - 1] == '_'){
					variableName[ len - 2] = 0;
				}
				strncpy( (void*)v.address, variableName + prefix, v.length);
			}
		break;
	case VAR_TYPE_STRUCT:
		while( PV_item( VariableName, memberIndex++, memberName) == 0 ){
			strcpy( variableName, VariableName);
			strncat( variableName,".", sizeof(variableName) );
			strncat( variableName, memberName, sizeof(variableName));
			if( varPopulateMemberNames( variableName, prefix ) ){
				break;
			}
		}
		break;	
	case VAR_TYPE_ARRAY_OF_STRUCT:
		for( memberIndex = 0; memberIndex < v.dimension; memberIndex++ ){
			strcpy( variableName, VariableName);
			strncat( variableName,"[", sizeof(variableName) );
			itoa(memberIndex, arrayIndex, 10);	
			strncat( variableName, arrayIndex, sizeof(variableName));
			strncat( variableName,"]", sizeof(variableName) );
			if( varPopulateMemberNames( variableName, prefix ) ){
				break;			
			}
		}
		break;	
	default:
		break;
	}

	return 0;

}
