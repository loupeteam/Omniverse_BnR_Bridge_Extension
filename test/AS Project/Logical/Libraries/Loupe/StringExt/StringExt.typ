(*
 * File: StringExt.typ
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of StringExt, licensed under the MIT License.
 *
 *)

TYPE
	STREXT_ERR_enum : 
		(
		STREXT_ERR_BUFFER_TOO_SMALL := -4, (*Destination buffer is too small for the result*)
		STREXT_ERR_RANGE := -3, (*Value does not fit in the destination type*)
		STREXT_ERR_INVALID_FORMAT := -2, (*String is not a valid literal of the expected form*)
		STREXT_ERR_INVALID_INPUT := -1
		);
	STREXT_HEXPREFIX_enum : 
		(
		STREXT_HEXPREFIX_NONE := 0, (*FF*)
		STREXT_HEXPREFIX_0X, (*0xFF*)
		STREXT_HEXPREFIX_IEC, (*16#FF*)
		STREXT_HEXPREFIX_DOLLAR (*$FF*)
		);
	StrExtArgs_typ : 	STRUCT  (*Arguments for formatter*)
		r : ARRAY[0..4]OF REAL; (*Reals*)
		s : ARRAY[0..4]OF UDINT; (*Pointers to String*)
		b : ARRAY[0..4]OF BOOL; (*Booleans*)
		i : ARRAY[0..4]OF DINT; (*Signed Integers*)
	END_STRUCT;
END_TYPE
