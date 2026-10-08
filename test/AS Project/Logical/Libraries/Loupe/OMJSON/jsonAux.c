/*
 * File: jsonAux.c
 * Copyright (c) 2009 - 2023 Loupe, Dave Gamble, and cJSON contributors 
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
	#include "vartools.h"
	#include "databuffer.h"
	#include <string.h>
	#include "jsonAux.h"
	#include "StringExt.h"

#ifdef __cplusplus
	};
#endif

//*******************************************************************
// Helper functions from cJSON library								*
//*******************************************************************

// TODO: These functions need maxLength inputs. Boo.

// Skip whitespace
char *skip(char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}

// Parse the input text into an unescaped cstring
char *parse_string(char *pDest, char *pSrc, unsigned long maxLength)
{
	
	char *ptr = pSrc + 1; // Ignore starting '\"' by starting + 1
	char *ptr2;
	unsigned long length = 0;
	//unsigned uc, uc2;
	
	if (pSrc == 0 || pDest == 0 || maxLength == 0) return 0;
	if (*pSrc != '\"') return 0; /* not a string! */
	
	//while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. Huh? */
	
	ptr = pSrc + 1;
	ptr2 = pDest;

	while (*ptr != '\"' && *ptr) {
		
		// Check length
		length = ptr2 - pDest;
		if (length >= maxLength) {
			// TODO: What should we do here?
			break;
		}
		
		// Copy characters while checking for escape sequences and ignoring invalid characters
		if (*ptr < 32 && !(*ptr >= 8 && *ptr <= 13 && *ptr != 11)) {
			// ignore invalid char
			ptr++;
		} else if (*ptr != '\\') {
			
			*ptr2++ = *ptr++; // Not an escape sequence. Copy char and move on.
		
		} else {
		
			ptr++;
			switch (*ptr) {
				case '\"': *ptr2++ = '\"';	break;
				case '\\': *ptr2++ = '\\';	break;
				case '/': *ptr2++ = '/';	break;
				case 'b': *ptr2++ = '\b';	break;
				case 'f': *ptr2++ = '\f';	break;
				case 'n': *ptr2++ = '\n';	break;
				case 'r': *ptr2++ = '\r';	break;
				case 't': *ptr2++ = '\t';	break;

				//				case 'u':	 /* transcode utf16 to utf8. */
				//					uc=parse_hex4(ptr+1);ptr+=4;	/* get the unicode char. */
				//
				//					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)	break;	/* check for invalid.	*/
				//
				//					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/
				//					{
				//						if (ptr[1]!='\\' || ptr[2]!='u')	break;	/* missing second-half of surrogate.	*/
				//						uc2=parse_hex4(ptr+3);ptr+=6;
				//						if (uc2<0xDC00 || uc2>0xDFFF)		break;	/* invalid second-half of surrogate.	*/
				//						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
				//					}
				//
				//					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
				//					
				//					switch (len) {
				//						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 1: *--ptr2 =(uc | firstByteMark[len]);
				//					}
				//					ptr2+=len;
				//					break;
				
				default: *ptr2++ = '\\'; *ptr2++ = *ptr; break;
				
			}
		
			ptr++;
		
		} // escape sequence
				
	} // loop through ptr string
	
	*ptr2 = 0; // null terminate ptr2 string
	if (*ptr == '\"') ptr++; // move past ptr string
	return ptr;
	
}

// Parse the input text into an unescaped wstring
char *parse_wstring(unsigned short *pDest, char *pSrc, unsigned long maxLength)
{
	
	char *ptr = pSrc + 1; // Ignore starting '\"' by starting + 1
	unsigned short *ptr2;
	unsigned long length = 0;
	//unsigned uc, uc2;
	
	if (pSrc == 0 || pDest == 0 || maxLength == 0) return 0;
	if (*pSrc != '\"') return 0; /* not a string! */
	
	//while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. Huh? */
	
	ptr = pSrc + 1;
	ptr2 = pDest;

	while (*ptr != '\"' && *ptr) {
		
		// Check length
		length = ptr2 - pDest;
		if (length >= maxLength) {
			// TODO: What should we do here?
			break;
		}
		
		// Copy characters while checking for escape sequences and ignoring invalid characters
		if (*ptr < 32 && !(*ptr >= 8 && *ptr <= 13 && *ptr != 11)) {
			// ignore invalid char
			ptr++;
		} else if (*ptr != '\\') {
			
			*ptr2++ = char2wchar(*ptr++); // Not an escape sequence. Copy char and move on.
		
		} else {
		
			ptr++;
			switch (*ptr) {
				case '\"': *ptr2++ = L'\"';	break;
				case '\\': *ptr2++ = L'\\';	break;
				case '/': *ptr2++ = L'/';	break;
				case 'b': *ptr2++ = L'\b';	break;
				case 'f': *ptr2++ = L'\f';	break;
				case 'n': *ptr2++ = L'\n';	break;
				case 'r': *ptr2++ = L'\r';	break;
				case 't': *ptr2++ = L'\t';	break;

				//				case 'u':	 /* transcode utf16 to utf8. */
				//					uc=parse_hex4(ptr+1);ptr+=4;	/* get the unicode char. */
				//
				//					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)	break;	/* check for invalid.	*/
				//
				//					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/
				//					{
				//						if (ptr[1]!='\\' || ptr[2]!='u')	break;	/* missing second-half of surrogate.	*/
				//						uc2=parse_hex4(ptr+3);ptr+=6;
				//						if (uc2<0xDC00 || uc2>0xDFFF)		break;	/* invalid second-half of surrogate.	*/
				//						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
				//					}
				//
				//					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
				//					
				//					switch (len) {
				//						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 1: *--ptr2 =(uc | firstByteMark[len]);
				//					}
				//					ptr2+=len;
				//					break;
				
				default: *ptr2++ = L'\\'; *ptr2++ = char2wchar(*ptr); break;
				
			}
		
			ptr++;
		
		} // escape sequence
				
	} // loop through ptr string
	
	*ptr2 = 0; // null terminate ptr2 string
	if (*ptr == '\"') ptr++; // move past ptr string
	return ptr;
	
}

// Unescape json string
// Returns new length of pDest
unsigned long unescape_string(char *pDest, char *pSrc, unsigned long maxLength, char **reentry)
{
	char *ptr;
	char *ptr2;
	unsigned long length = 0;
	//unsigned uc, uc2;
	
	if (pSrc == 0 || pDest == 0 || maxLength == 0) return 0;
	
	ptr = pSrc;
	ptr2 = pDest;
	
	if(reentry != 0)
		*reentry = 0; // Reset reentry pointer to NULL

	while (*ptr) {
		
		// Check length
		length = ptr2 - pDest;
		if (length >= maxLength) {
			if(reentry != 0)
				*reentry = ptr; // Copy out stopped position so we can reenter
			break;
		}
		
		// Check for invalid whitespace characters
		if (*ptr < 32 && !(*ptr >= 8 && *ptr <= 13 && *ptr != 11)) {
			strcpy(pDest, "");
			return 0;
		}
		
		// Copy characters while checking for escape sequences
		if (*ptr != '\\') {
			
			*ptr2++ = *ptr++; // Not an escape sequence. Copy char and move on.
		
		} else {
		
			ptr++;
			switch (*ptr) {
				case '\"': *ptr2++ = '\"';	break;
				case '\\': *ptr2++ = '\\';	break;
				case '/': *ptr2++ = '/';	break;
				case 'b': *ptr2++ = '\b';	break;
				case 'f': *ptr2++ = '\f';	break;
				case 'n': *ptr2++ = '\n';	break;
				case 'r': *ptr2++ = '\r';	break;
				case 't': *ptr2++ = '\t';	break;
				
				default: *ptr2++ = '\\'; *ptr2++ = *ptr; break;
				
			}
		
			ptr++;
		
		} // escape sequence
				
	} // loop through ptr string
	
	*ptr2 = 0; // null terminate ptr2 string
	return (ptr2 - pDest); // strlen of pDest
}

// Stringify the input text into an escaped JSON string
// Retruns new length of pDest
unsigned long stringify_string(char *pDest, char *pSrc, unsigned long maxLength, char** reentry)
{
	
	char *ptr;
	char *ptr2;
	unsigned long length = 0;
	//unsigned uc, uc2;
	
	if (pSrc == 0 || pDest == 0 || maxLength == 0) return 0;
	
	ptr = pSrc;
	ptr2 = pDest;
	
	if(reentry != 0)
		*reentry = 0; // Reset reentry pointer to NULL

	while (*ptr) {
		
		// Check length
		length = ptr2 - pDest;
		if (length >= (maxLength - 1)) { // We can add 2 characters per a iteration so check maxLength - 1 
			if(reentry != 0)
				*reentry = ptr; // Copy out stopped position so we can reenter
			break;
		}
		
		// Check for invalid whitespace characters
		if (*ptr < 32 && *ptr > 0 && !(*ptr >= 8 && *ptr <= 13 && *ptr != 11)) {
			length = stringlcpy((UDINT)pDest, (UDINT)"Invalid String", maxLength+1);
			length = length > maxLength ? maxLength : length;
			return length;
		}
		
		if(*ptr < 0x7F) {
		
			// Copy characters while checking for characters that should be escaped
			switch (*ptr) {
				case '\"': *ptr2++ = '\\'; *ptr2++ = '\"'; break;
				case '\\': *ptr2++ = '\\'; *ptr2++ = '\\'; break;
				case '\b': *ptr2++ = '\\'; *ptr2++ = 'b'; break;
				case '\f': *ptr2++ = '\\'; *ptr2++ = 'f'; break;
				case '\n': *ptr2++ = '\\'; *ptr2++ = 'n'; break;
				case '\r': *ptr2++ = '\\'; *ptr2++ = 'r'; break;
				case '\t': *ptr2++ = '\\'; *ptr2++ = 't'; break;

				//				case '\u':	 /* transcode utf16 to utf8. */
				//					uc=parse_hex4(ptr+1);ptr+=4;	/* get the unicode char. */
				//
				//					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)	break;	/* check for invalid.	*/
				//
				//					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/
				//					{
				//						if (ptr[1]!='\\' || ptr[2]!='u')	break;	/* missing second-half of surrogate.	*/
				//						uc2=parse_hex4(ptr+3);ptr+=6;
				//						if (uc2<0xDC00 || uc2>0xDFFF)		break;	/* invalid second-half of surrogate.	*/
				//						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
				//					}
				//
				//					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
				//					
				//					switch (len) {
				//						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
				//						case 1: *--ptr2 =(uc | firstByteMark[len]);
				//					}
				//					ptr2+=len;
				//					break;
			
				default:  *ptr2++ = *ptr; break;		
			}
		}
		else { // multi-byte character, assume unicode 
			if(*ptr & 0x11100000 == 0x11000000) { // 2 bytes
				*ptr2++ = *ptr;
				ptr++;
				*ptr2++ = *ptr;
			}
			else if(*ptr & 0x11110000 == 0x11100000) {// 3 bytes
				if(length < (maxLength - 2)) {
					*ptr2++ = *ptr;
					ptr++;
					*ptr2++ = *ptr;
					ptr++;
					*ptr2++ = *ptr;
				}
			}
			else if(*ptr & 0x11111000 == 0x11110000) {// 4 bytes
				if(length < (maxLength - 2)) {
					*ptr2++ = *ptr;
					ptr++;
					*ptr2++ = *ptr;
					ptr++;
					*ptr2++ = *ptr;
					ptr++;
					*ptr2++ = *ptr;
				}
			}
		}
		
		ptr++;
		
	} // loop through ptr string
	
	*ptr2 = 0; // null terminate ptr2 string
	return (ptr2 - pDest); // strlen of pDest
	
}

// Stringify the input text into an escaped JSON string
// Retruns new length of pDest
unsigned long stringify_wstring(char *pDest, unsigned short *pSrc, unsigned long maxLength, unsigned short** reentry)
{
	
	unsigned short *ptr;
	char *ptr2;
	unsigned long length = 0;
	unsigned short tempWChar;
	//unsigned uc, uc2;
	
	if (pSrc == 0 || pDest == 0 || maxLength == 0) return 0;
	
	ptr = pSrc;
	ptr2 = pDest;
	
	if(reentry != 0)
		*reentry = 0; // Reset reentry pointer to NULL

	while (*ptr) {
		
		// Check length
		length = ptr2 - pDest;
		if (length >= (maxLength - 1)) { // We can add 2 characters per a iteration so check maxLength - 1 
			if(reentry != 0)
				*reentry = ptr; // Copy out stopped position so we can reenter
			break;
		}
		
		// Check for invalid whitespace characters
		tempWChar = wchar2char(*ptr);
		if (tempWChar < 32 && !(tempWChar >= 8 && tempWChar <= 13 && tempWChar != 11)) {
			length = stringlcpy((UDINT)pDest, (UDINT)"Invalid String", maxLength+1);
			length = length > maxLength ? maxLength : length;
			return length;
		}
		
		// Copy characters while checking for characters that should be escaped
		switch (*ptr) {
			case L'\"': *ptr2++ = '\\'; *ptr2++ = '\"'; break;
			case L'\\': *ptr2++ = '\\'; *ptr2++ = '\\'; break;
			case L'\b': *ptr2++ = '\\'; *ptr2++ = 'b'; break;
			case L'\f': *ptr2++ = '\\'; *ptr2++ = 'f'; break;
			case L'\n': *ptr2++ = '\\'; *ptr2++ = 'n'; break;
			case L'\r': *ptr2++ = '\\'; *ptr2++ = 'r'; break;
			case L'\t': *ptr2++ = '\\'; *ptr2++ = 't'; break;
			
			default:  *ptr2++ = wchar2char(*ptr); break;		
		}
		
		ptr++;
		
	} // loop through ptr string
	
	*ptr2 = 0; // null terminate ptr2 string
	return (ptr2 - pDest); // strlen of pDest
	
}

// Parse the input text into a number string
char *parse_number(char *pDest, char *pSrc)
{
	if( pSrc == 0 || pDest == 0 ) return 0;
	
	char *ptr, *ptr2;
	ptr = pSrc;
	ptr2 = pDest;
	
	while(	*ptr!=',' 
		&& 	*ptr!=']' 
		&& 	*ptr!='}' 
		&& 	*ptr!=' ' 
		&& 	*ptr!='\n'
		&& 	*ptr!='\r'
		&& 	*ptr!='\t'
		&& 	*ptr)
	{
		*ptr2++ = *ptr++;
	} // loop through ptr string
	
	*ptr2 = 0; // null terminate ptr2 string
	return ptr;
}


// Parser core - when encountering text, process appropriately
char *parse_value(varVariable_typ *pVariable, char *value)
{

	if( pVariable == 0 || value == 0 ) return 0;
	
	if( !strncmp(value,"null",4) ) return value+4;
	
	if( !strncmp(value,"undefined",9) ) return value+9;
	
	varGetInfo( (UDINT)pVariable );
	
	if( !strncmp(value,"false",5) ){
		strcpy( pVariable->value, "false" );
		varSetValue( (UDINT)pVariable );
		return value+5; 
	}
	
	if( !strncmp(value,"true",4) ){	
		strcpy( pVariable->value, "true" );
		varSetValue( (UDINT)pVariable );
		return value+4;
	}
	
	if (*value == '\"') {
		if (pVariable->dataType == VAR_TYPE_STRING) {
			return parse_string((char*)pVariable->address, value, pVariable->length - 1);
		}
		else if(pVariable->dataType == VAR_TYPE_WSTRING) {
			return parse_wstring((unsigned short*)pVariable->address, value, pVariable->length - 2); // Minus 2 because a WString null char is 2 bytes
		} else {
			value = parse_string((char*)pVariable->value, value, sizeof(pVariable->value) - 1);
			varSetValue((UDINT)pVariable);
			return value;
		}
	}
	
	if (*value=='-' || (*value>='0' && *value<='9') ){
		value = parse_number(pVariable->value, value);
		varSetValue( (UDINT)pVariable );
		return value;
	}
	
	if( *value=='[' || *value=='{' ) return value;
	
	return value;
	
} // parse_value

unsigned short varValueToJsonString(varVariable_typ *pVariable, datbufBuffer_typ *buffer)
{
	unsigned short appendStatus = 0;
	// Append value
	switch (pVariable->dataType)
	{

	case VAR_TYPE_STRING:
	{
		// Stringify - Do not use variable.value. It is limited to 120 characters, which is not enough
		// for strings.
		// TODO: This can cause a stack overflow if variable.length is very large.
		//		Maybe add a max size
		STRING tempString[pVariable->length + 50];
		char *reentry;
		stringify_string(tempString, (char *)pVariable->address, sizeof(tempString) - 1, &reentry);

		// Append ""tempString""
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\""), 1);
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT)&tempString, strlen(tempString));
		if (reentry != 0)
		{
			stringify_string(tempString, reentry, sizeof(tempString) - 1, &reentry);
			appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT)&tempString, strlen(tempString));
		}
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\""), 1);
	}
	break;

	case VAR_TYPE_WSTRING:
	{
		// Stringify - Do not use variable.value. It is limited to 120 characters, which is not enough
		// for strings.
		// TODO: This can cause a stack overflow if variable.length is very large.
		//		Maybe add a max size
		char tempString[pVariable->length + 50];
		// NOTE: Because length is in bytes, tempString will contain 2x the number of characters
		// We can change this to length/2 but if we do support UTF-8 in the future this will need to be just length
		// This is because WSTRING to UTF-8 may result in the same number of characters
		WSTRING *reentry;
		stringify_wstring(tempString, (UINT *)pVariable->address, sizeof(tempString) - 1, &reentry);

		// Append ""tempString""
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\""), 1);
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT)&tempString, strlen(tempString));
		if (reentry != 0)
		{
			stringify_wstring(tempString, reentry, sizeof(tempString) - 1, &reentry);
			appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT)&tempString, strlen(tempString));
		}
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\""), 1);
	}
	break;

	case VAR_TYPE_REAL:
	case VAR_TYPE_LREAL:
	{
		// TODO: Test this for performance
		REAL value;

		if (pVariable->dataType == VAR_TYPE_LREAL)
		{
			value = (REAL) * (LREAL *)(pVariable->address);
		}
		else
		{
			value = *(REAL *)(pVariable->address);
		}

		if (isnan(value))
		{

			// Append ""NaN""
			appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\"NaN\""), 5); // 5 = strlen("\"NaN\"")
		}
		else if (isinf(value))
		{

			if (value > 0)
			{
				// Append ""Infinity""
				appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\"Infinity\""), 10); // 10 = strlen("\"Infinity\"")
			}
			else
			{
				// Append ""-Infinity""
				appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\"-Infinity\""), 11); // 11 = strlen("\"-Infinity\"")
			}
		}
		else
		{

			// Append value
			appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & (pVariable->value), strlen(pVariable->value));
		}
	}
	break;

	case VAR_TYPE_UNDEFINED:

		// Append ""undefined""
		appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\"undefined\""), 11); // 11 = strlen("\"undefined\"")

		break;

	default:

		if (pVariable->value[0] != '\0')
		{
			// Append value
			appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & (pVariable->value), strlen(pVariable->value));
		}
		else
		{
			// Type or value is not supported
			// We need to provide something to be valid json
			// Append dummy value: ""undefined""
			appendStatus = datbufAppendToBuffer((UDINT) buffer, (UDINT) & ("\"undefined\""), 11); // 11 = strlen("\"undefined\"")
		}
		break;

	} // switch(dataType)

	return appendStatus;
}