/********************************************************************
 * COPYRIGHT -- Microsoft
 ********************************************************************
 * Library: StringExt
 * File: Timestamp_TO_DT.c
 * Author: ScismD
 * Created: April 20, 2015
 ********************************************************************
 * Implementation of library StringExt
 ********************************************************************/

#include <bur/plctypes.h>
#include <string.h>
#ifdef __cplusplus
  extern "C"
  {
#endif

  #include "StringExt.h"

#ifdef __cplusplus
  };
#endif


#include <string.h>
#include <stdlib.h>


// Converts output of GenerateTimestamp to DATE_AND_TIME

// DONE not tested

plcdt Timestamp_TO_DT(unsigned long pDT, unsigned long pTimestamp)
{
 
	// Check for null pointer
  if(pTimestamp == 0)
  {
    return 0;  
  } 

  UINT        Length;
  STRING      TempString[6];
  DTStructure    DTStruct;
 
  Length = strlen((char*)pTimestamp);
  if (Length < 19)//19 is the size of the GenerateTimestamp function.
  {
    memset(&DTStruct,0,sizeof(DTStruct));
    return 0;
  }

  //clear string
  memset(&TempString,0,sizeof(TempString));

  //year
  memcpy(&TempString,(char*)pTimestamp,4);
  DTStruct.year = atoi(TempString);

  //clear string
  memset(&TempString,0,sizeof(TempString));

  //month
  memcpy(&TempString,(char*)pTimestamp+5,2);
  DTStruct.month = atoi(TempString);

  //clear string
  memset(&TempString,0,sizeof(TempString));

  //day
  memcpy(&TempString,(char*)pTimestamp+8,2);
  DTStruct.day = atoi(TempString);

  //clear string
  memset(&TempString,0,sizeof(TempString));

  //hour
  memcpy(&TempString,(char*)pTimestamp+11,2);
  DTStruct.hour = atoi(TempString);

  //clear string
  memset(&TempString,0,sizeof(TempString));

  //minute
  memcpy(&TempString,(char*)pTimestamp+14,2);
  DTStruct.minute = atoi(TempString);

  //clear string
  memset(&TempString,0,sizeof(TempString));

  //second
  memcpy(&TempString,(char*)pTimestamp+17,2);
  DTStruct.second = atoi(TempString);

  DTStruct.wday = 0;//this needs to be here otherwise the DTStruct to DT fails.
  DTStruct.millisec = 0;
  DTStruct.microsec = 0;
 
  if (pDT != 0)
  {
    memcpy((char*)pDT,&DTStruct,sizeof(DTStruct));
  }

  return DTStructure_TO_DT((UDINT)&DTStruct);
}
