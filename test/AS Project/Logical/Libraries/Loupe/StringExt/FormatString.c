/*
 * File: FormatString.c
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

#include "string.h"

// Define some basic macros 
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef ARRAY_ENTRIES
#define ARRAY_ENTRIES(array) (sizeof(array)/sizeof(array[0]))
#endif

/*
 * 
 */
signed long formatString(plcstring* destination, unsigned long destSize, plcstring* src, StrExtArgs_typ* pArguments) {
	
	// Check for invalid inputs
	if(!destination || !src || !pArguments)
		return STREXT_ERR_INVALID_INPUT;

	// Temporary array for string concatenations 
	char temp_string[16];

	// Size of a string that is concatenated to the destination
	int concat_string_size;
	
	char* dst = destination;
	
	// Array index counters 
	int bool_counter   = 0;		
	int int_counter    = 0;
	int real_counter   = 0;
	int string_counter = 0;

	// If size is 0, dont do anything
	// We can't do anything in that case
	if(!destSize)
		return 0;
		
	destSize--; // Remove 1 from dest size to leave room for null char
	
	// Run through the source message and copy it to the destination until src is empty or destination is full 
	while ((*src != '\0') && (destSize > 0))
	{
		// Special character found and MessageData given 
		if ((pArguments != NULL) && (*src == '%'))
		{
			// Temporary 'close' the destination string (to allow strcat...) 
			*dst = '\0';
		
			// Consume the char and investigate the command 
			src++;
			
			// A '%' at the very end of the source has no command character.
			// Leave the loop here - the cases below would step past the terminator.
			if (*src == '\0')
				break; // exits the while loop, not a switch
			
			switch (*src)
			{
				// Insert bool 
				case 'b':
					// Only take a value if an array entry exists 
					if (bool_counter < ARRAY_ENTRIES(pArguments->b))
					{
						if (pArguments->b[bool_counter])
						{
							strncat(dst, "TRUE", destSize);
							concat_string_size = MIN(destSize, 4);
						}
						else
						{
							strncat(dst, "FALSE", destSize);
							concat_string_size = MIN(destSize, 5);
						}
						bool_counter++;
						dst += concat_string_size;
						destSize -= concat_string_size;
					}
					src++;
					break;
					
				// Insert integer 
				case 'i':
				case 'd':
					// Only take a value if an array entry exists 
					if (int_counter < ARRAY_ENTRIES(pArguments->i))
					{
						concat_string_size = brsitoa(pArguments->i[int_counter], (UDINT)temp_string);
						concat_string_size = MIN(destSize, concat_string_size);
						strncat(dst, temp_string, destSize);
						int_counter++;
						dst += concat_string_size;
						destSize -= concat_string_size;
					}
					src++;
					break;
					
				// Insert real 
				case 'r':
				case 'f':
					// Only take a value if an array entry exists 
					if (real_counter < ARRAY_ENTRIES(pArguments->r))
					{
						concat_string_size = brsftoa(pArguments->r[real_counter], (UDINT)temp_string);
						concat_string_size = MIN(destSize, concat_string_size);
						strncat(dst, temp_string, destSize);
						real_counter++;
						dst += concat_string_size;
						destSize -= concat_string_size;
					}
					src++;
					break;
					
				// Insert string 
				case 's':
					// Only take a value if an array entry exists 
					if (string_counter < ARRAY_ENTRIES(pArguments->s))
					{
						if (pArguments->s[string_counter]) 
						{
							strncat(dst, (char*)pArguments->s[string_counter], destSize);
							concat_string_size = MIN(destSize, strlen((char*)pArguments->s[string_counter]));
							dst += concat_string_size;
							destSize -= concat_string_size;
						}
						string_counter++;
					}
					src++;
					break;
					
				// Insert percent 
				case '%':
					*dst++ = '%';
					src++;
					destSize--;
					break;
					
				// ELSE 
				default:
					// Ignore the character 
					src++;
					break;
			}
		}
		else
		{
			*dst++ = *src++;
			destSize--;
		}
	}
	
	// Add the end of string 
	*dst = '\0';
	
	return dst - destination;
}
