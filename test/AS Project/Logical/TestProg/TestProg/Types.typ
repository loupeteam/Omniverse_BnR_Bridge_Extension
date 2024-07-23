
TYPE
	mySecondStruct_typ : 	STRUCT 
		bool : BOOL;
		array : ARRAY[0..5]OF USINT;
	END_STRUCT;
	myStruct : 	STRUCT 
		var1 : USINT;
		var2 : USINT;
		secondStruct : mySecondStruct_typ;
	END_STRUCT;
END_TYPE
