(*
 * File: Hammers.fun
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of Hammers, licensed under the MIT License.
 * 
  ********************************************************************
 * Functions and function blocks of library Hammers
 ********************************************************************)

FUNCTION crc_16 : UINT
	VAR_INPUT
		input_str : UDINT;
		num_bytes : UDINT;
	END_VAR
END_FUNCTION

FUNCTION_BLOCK AccumulateDistance
	VAR_INPUT
		enable : BOOL;
		input : REAL; (*Periodic input value*)
		period : REAL; (*Period of input value*)
		pDistance : UDINT;
	END_VAR
	VAR_OUTPUT
		initialized : BOOL;
		outputREAL : REAL; (*Accumulated output*)
		outputLREAL : LREAL; (*Accumulated output*)
		outputCYCLIC_POSITION : hamCYCLIC_POSITION; (*Accumulated output*)
	END_VAR
	VAR
		inputOld : REAL; (*Last value of input for calculating delta*)
		delta : LREAL; (*Change in value in last scan, corrected for rollover*)
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK Accumulate (*Accumulate a periodic input (un-modulo)*) (*$GROUP=User*)
	VAR_INPUT
		input : REAL; (*Periodic input value*)
		period : REAL; (*Period of input value*)
		setToInput : BOOL; (*Force the input to the output and reinitialize accumulator*)
	END_VAR
	VAR_OUTPUT
		outputREAL : REAL; (*Accumulated output*)
		outputLREAL : LREAL; (*Accumulated output*)
		outputCYCLIC_POSITION : hamCYCLIC_POSITION; (*Accumulated output*)
	END_VAR
	VAR
		initialized : BOOL; (*Function block has been initialized*)
		inputOld : REAL; (*Last value of input for calculating delta*)
		delta : LREAL; (*Change in value in last scan, corrected for rollover*)
		numPeriods : DINT; (*Signed number of periods the input has gone through*)
		accumulatedValue_add : LREAL; (*Accumulated output computed by (accumulatedValue_add + delta)*)
		accumulatedValue_mult : LREAL; (*Accumulated output computed by (numPeriods * period + input)*)
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK ScaleAnalogInput (*Scale an analog input value from counts to units*) (*$GROUP=User*)
	VAR_INPUT
		enable : BOOL;
		input : INT;
		configuration : hamAIO_CONFIG;
		V : ARRAY[0..HAM_MAI_SCALE_POINTS] OF REAL;
		mA : ARRAY[0..HAM_MAI_SCALE_POINTS] OF REAL;
		units : ARRAY[0..HAM_MAI_SCALE_POINTS] OF REAL;
		numberOfPoints : USINT;
		update : BOOL;
	END_VAR
	VAR_OUTPUT
		output : REAL;
		status : DINT;
		updateDone : BOOL;
	END_VAR
	VAR
		internal : hamScaleAI_internal_typ;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION hamTempSensorFn : BOOL
	VAR_IN_OUT
		t : hamTempSensor_typ;
	END_VAR
END_FUNCTION

FUNCTION subtractWithRollover : REAL (*Subtract two REALs and return the difference, corrected for rollover*) (*$GROUP=User*)
	VAR_INPUT
		minuend : REAL;
		subtrahend : REAL;
		rolloverPeriod : REAL;
	END_VAR
END_FUNCTION

FUNCTION Mean : LREAL (*Return the mean of a data set*) (*$GROUP=User*)
	VAR_INPUT
		pData : UDINT;
		n : UDINT;
	END_VAR
END_FUNCTION

FUNCTION StandardDeviation : LREAL (*Compute the standard deviation of a data set*) (*$GROUP=User*)
	VAR_INPUT
		pData : UDINT;
		n : UDINT;
	END_VAR
END_FUNCTION

FUNCTION_BLOCK Blink (*TODO: Add your comment here*) (*$GROUP=User,$CAT=User,$GROUPICON=User.png,$CATICON=User.png*)
	VAR_INPUT
		Enable : BOOL;
		Time : DINT;
		Percentage : UDINT;
	END_VAR
	VAR_OUTPUT
		Out : BOOL;
	END_VAR
	VAR
		TimerOff : TON;
		TimerOn : TON;
		onTime : UDINT;
		offTime : UDINT;
		_RTInfo : RTInfo;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK Interval
	VAR_INPUT
		IN : BOOL;
		PT : UDINT; (*[us]*)
	END_VAR
	VAR_OUTPUT
		ET : UDINT; (*[us]*)
		Q : BOOL;
	END_VAR
	VAR
		start : UDINT;
		started : BOOL;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK FastTON
	VAR_INPUT
		IN : BOOL;
		PT : UDINT; (*[us]*)
	END_VAR
	VAR_OUTPUT
		ET : UDINT; (*[us]*)
		Q : BOOL;
	END_VAR
	VAR
		start : UDINT;
		started : BOOL;
	END_VAR
END_FUNCTION_BLOCK

{REDUND_ERROR} FUNCTION_BLOCK SettleDown (*TODO: Add your comment here*) (*$GROUP=User,$CAT=User,$GROUPICON=User.png,$CATICON=User.png*)
	VAR_INPUT
		in : BOOL;
		onTime : TIME;
		offTime : TIME;
	END_VAR
	VAR_OUTPUT
		out : BOOL;
	END_VAR
	VAR
		internal : SdInternal_typ;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION SimulationStatus : BOOL (*Used to find the Simulation status for your project*) (*$GROUP=User,$CAT=User,$GROUPICON=User.png,$CATICON=User.png*)
	VAR_INPUT
		pOverride :  UDINT; (**)
	END_VAR
END_FUNCTION
