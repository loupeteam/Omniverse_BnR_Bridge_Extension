/*
 * File: SplitFileName.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of StringExt, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif
	#include "StringExt.h"
#ifdef __cplusplus
	};
#endif

#include<string.h>

/* Seperates file extension from file name */
unsigned long SplitFileName(unsigned long pFileName, unsigned long pName, unsigned long pExtension)
{
	UDINT strLength;
	UDINT nameLength;
	UDINT extLength = 0;
	char * name;
	
	// Check input
	if(pFileName == 0)
		return 0;
	
	// Get file name length
	strLength = nameLength = strlen((char*)pFileName);
	
	// Find Name
	name = strrchr((char*)pFileName, '.');
	
	if(name != NULL)
		nameLength = name - ((char*)pFileName);
	
	// Find Extension
	extLength = (strLength - nameLength);
	if(extLength > 0)
		extLength = extLength - 1;
	
	STRING tempString[strLength+1];
	
	memcpy(&tempString, (void*)pFileName, strLength+1);
	
	// Fill extension
	if(pExtension != 0) {
		memcpy((void*)pExtension, (void*)(((char*)&tempString) + nameLength + 1), extLength);
		((char*)pExtension)[extLength] = '\0';
	}
	
	// Fill name
	if(pName != 0) {
		memcpy((void*)pName,&tempString,nameLength);
		((char*)pName)[nameLength] = '\0';
	}
	
	// Return length of name
	return nameLength;
}
