/*
 * File: time.cpp
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

#include "./gdtoa/gdtoa.h"


unsigned long stringdtoa(double value, plcstring* buffer, unsigned long ndig, unsigned long bufferSize) {	
	return g_dfmt(buffer, &value, ndig, bufferSize);
}
unsigned long stringftoa(float value, plcstring* buffer, unsigned long ndig, unsigned long bufferSize) {	
	return g_ffmt(buffer, &value, ndig, bufferSize);
}

double stringstrtod(plcstring* value, plcstring** pEnd) {
	return strtod(value, pEnd);
}
float stringstrtof(plcstring* value, plcstring** pEnd) {
	return strtof(value, pEnd);
}