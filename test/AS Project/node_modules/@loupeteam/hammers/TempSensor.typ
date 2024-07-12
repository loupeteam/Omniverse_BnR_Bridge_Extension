(*
 * File: TempSensor.typ
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of Hammers, licensed under the MIT License.
 * 
 *)


TYPE
	TEMP_LEVEL_enum : 
		(
		TEMP_LEVEL_OK,
		TEMP_LEVEL_WARNING,
		TEMP_LEVEL_HIGH
		);
	hamTempSensor_OUT_typ : 	STRUCT 
		Temperature : REAL; (*[�C]*)
		TempLevel : TEMP_LEVEL_enum;
		HighTempAlarm : BOOL;
		WarningTempAlarm : BOOL;
	END_STRUCT;
	hamTempSensor_IN_PAR_typ : 	STRUCT 
		aiValue : INT;
	END_STRUCT;
	hamTempSensor_IN_CFG_typ : 	STRUCT 
		ScaleFactor : REAL; (*default [0.1�C]*)
		HighTemp : REAL; (*[�C]*)
		WarningPercentage : REAL; (*default 85% [%]*)
		HighPercentage : REAL; (*default 95% [%]*)
	END_STRUCT;
	hamTempSensor_IN_typ : 	STRUCT 
		PAR : hamTempSensor_IN_PAR_typ;
		CFG : hamTempSensor_IN_CFG_typ;
	END_STRUCT;
	hamTempSensor_typ : 	STRUCT 
		IN : hamTempSensor_IN_typ;
		OUT : hamTempSensor_OUT_typ;
	END_STRUCT;
END_TYPE
