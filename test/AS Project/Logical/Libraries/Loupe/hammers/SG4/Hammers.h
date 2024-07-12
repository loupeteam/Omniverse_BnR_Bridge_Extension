/* Automation Studio generated header file */
/* Do not edit ! */
/* Hammers 0.10.0 */

#ifndef _HAMMERS_
#define _HAMMERS_
#ifdef __cplusplus
extern "C" 
{
#endif
#ifndef _Hammers_VERSION
#define _Hammers_VERSION 0.10.0
#endif

#include <bur/plctypes.h>

#ifndef _BUR_PUBLIC
#define _BUR_PUBLIC
#endif
#ifdef _SG3
		#include "AsIOTime.h"
		#include "standard.h"
		#include "MTLookUp.h"
#endif
#ifdef _SG4
		#include "AsIOTime.h"
		#include "standard.h"
		#include "MTLookUp.h"
#endif
#ifdef _SGC
		#include "AsIOTime.h"
		#include "standard.h"
		#include "MTLookUp.h"
#endif


/* Constants */
#ifdef _REPLACE_CONST
 #define HAM_NUM_SCALE_POINTS 50U
 #define HAM_MAI_SCALE_POINTS 49U
#else
 _GLOBAL_CONST unsigned char HAM_NUM_SCALE_POINTS;
 _GLOBAL_CONST unsigned char HAM_MAI_SCALE_POINTS;
#endif




/* Datatypes and datatypes of function blocks */
typedef enum hamAIO_CONFIG
{	hamAIO_CONFIG_V,
	hamAIO_CONFIG_0_20_MA,
	hamAIO_CONFIG_4_20_MA
} hamAIO_CONFIG;

typedef enum SdState_enum
{	ST_SD_IDLE,
	ST_SD_DEBOUNCE
} SdState_enum;

typedef enum TEMP_LEVEL_enum
{	TEMP_LEVEL_OK,
	TEMP_LEVEL_WARNING,
	TEMP_LEVEL_HIGH
} TEMP_LEVEL_enum;

typedef struct hamCYCLIC_POSITION
{	signed long Integer;
	float Real;
} hamCYCLIC_POSITION;

typedef struct hamScaleAI_internal_typ
{	struct MTLookUpTable mtLookup_ctsToVmA;
	struct MTLookUpTable mtLookup_VmAToUnits;
} hamScaleAI_internal_typ;

typedef struct SdFB_typ
{	struct TON timer;
} SdFB_typ;

typedef struct SdInternal_typ
{	struct SdFB_typ fb;
	plcbit _in;
	enum SdState_enum state;
} SdInternal_typ;

typedef struct hamTempSensor_OUT_typ
{	float Temperature;
	enum TEMP_LEVEL_enum TempLevel;
	plcbit HighTempAlarm;
	plcbit WarningTempAlarm;
} hamTempSensor_OUT_typ;

typedef struct hamTempSensor_IN_PAR_typ
{	signed short aiValue;
} hamTempSensor_IN_PAR_typ;

typedef struct hamTempSensor_IN_CFG_typ
{	float ScaleFactor;
	float HighTemp;
	float WarningPercentage;
	float HighPercentage;
} hamTempSensor_IN_CFG_typ;

typedef struct hamTempSensor_IN_typ
{	struct hamTempSensor_IN_PAR_typ PAR;
	struct hamTempSensor_IN_CFG_typ CFG;
} hamTempSensor_IN_typ;

typedef struct hamTempSensor_typ
{	struct hamTempSensor_IN_typ IN;
	struct hamTempSensor_OUT_typ OUT;
} hamTempSensor_typ;

typedef struct AccumulateDistance
{
	/* VAR_INPUT (analog) */
	float input;
	float period;
	unsigned long pDistance;
	/* VAR_OUTPUT (analog) */
	float outputREAL;
	double outputLREAL;
	struct hamCYCLIC_POSITION outputCYCLIC_POSITION;
	/* VAR (analog) */
	float inputOld;
	double delta;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR_OUTPUT (digital) */
	plcbit initialized;
} AccumulateDistance_typ;

typedef struct Accumulate
{
	/* VAR_INPUT (analog) */
	float input;
	float period;
	/* VAR_OUTPUT (analog) */
	float outputREAL;
	double outputLREAL;
	struct hamCYCLIC_POSITION outputCYCLIC_POSITION;
	/* VAR (analog) */
	float inputOld;
	double delta;
	signed long numPeriods;
	double accumulatedValue_add;
	double accumulatedValue_mult;
	/* VAR_INPUT (digital) */
	plcbit setToInput;
	/* VAR (digital) */
	plcbit initialized;
} Accumulate_typ;

typedef struct ScaleAnalogInput
{
	/* VAR_INPUT (analog) */
	signed short input;
	enum hamAIO_CONFIG configuration;
	float V[50];
	float mA[50];
	float units[50];
	unsigned char numberOfPoints;
	/* VAR_OUTPUT (analog) */
	float output;
	signed long status;
	/* VAR (analog) */
	struct hamScaleAI_internal_typ internal;
	/* VAR_INPUT (digital) */
	plcbit enable;
	plcbit update;
	/* VAR_OUTPUT (digital) */
	plcbit updateDone;
} ScaleAnalogInput_typ;

typedef struct Blink
{
	/* VAR_INPUT (analog) */
	signed long Time;
	unsigned long Percentage;
	/* VAR (analog) */
	struct TON TimerOff;
	struct TON TimerOn;
	unsigned long onTime;
	unsigned long offTime;
	struct RTInfo _RTInfo;
	/* VAR_INPUT (digital) */
	plcbit Enable;
	/* VAR_OUTPUT (digital) */
	plcbit Out;
} Blink_typ;

typedef struct Interval
{
	/* VAR_INPUT (analog) */
	unsigned long PT;
	/* VAR_OUTPUT (analog) */
	unsigned long ET;
	/* VAR (analog) */
	unsigned long start;
	/* VAR_INPUT (digital) */
	plcbit IN;
	/* VAR_OUTPUT (digital) */
	plcbit Q;
	/* VAR (digital) */
	plcbit started;
} Interval_typ;

typedef struct FastTON
{
	/* VAR_INPUT (analog) */
	unsigned long PT;
	/* VAR_OUTPUT (analog) */
	unsigned long ET;
	/* VAR (analog) */
	unsigned long start;
	/* VAR_INPUT (digital) */
	plcbit IN;
	/* VAR_OUTPUT (digital) */
	plcbit Q;
	/* VAR (digital) */
	plcbit started;
} FastTON_typ;

typedef struct SettleDown
{
	/* VAR_INPUT (analog) */
	plctime onTime;
	plctime offTime;
	/* VAR (analog) */
	struct SdInternal_typ internal;
	/* VAR_INPUT (digital) */
	plcbit in;
	/* VAR_OUTPUT (digital) */
	plcbit out;
} SettleDown_typ;



/* Prototyping of functions and function blocks */
_BUR_PUBLIC void AccumulateDistance(struct AccumulateDistance* inst);
_BUR_PUBLIC void Accumulate(struct Accumulate* inst);
_BUR_PUBLIC void ScaleAnalogInput(struct ScaleAnalogInput* inst);
_BUR_PUBLIC void Blink(struct Blink* inst);
_BUR_PUBLIC void Interval(struct Interval* inst);
_BUR_PUBLIC void FastTON(struct FastTON* inst);
_BUR_PUBLIC void SettleDown(struct SettleDown* inst);
_BUR_PUBLIC unsigned short crc_16(unsigned long input_str, unsigned long num_bytes);
_BUR_PUBLIC plcbit hamTempSensorFn(struct hamTempSensor_typ* t);
_BUR_PUBLIC float subtractWithRollover(float minuend, float subtrahend, float rolloverPeriod);
_BUR_PUBLIC double Mean(unsigned long pData, unsigned long n);
_BUR_PUBLIC double StandardDeviation(unsigned long pData, unsigned long n);
_BUR_PUBLIC plcbit SimulationStatus(unsigned long pOverride);


#ifdef __cplusplus
};
#endif
#endif /* _HAMMERS_ */

