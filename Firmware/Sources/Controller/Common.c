// Header
#include "Common.h"

// Includes
#include "HighLevelInterface.h"
#include "DataTable.h"

// Functions
//
Boolean CMN_UpdateNodeState(Boolean Emulate, Int16U NodeIDReg, pInt16U StateStorage)
{
	Int16U Register;
	Boolean Result = TRUE;

	if(!Emulate)
	{
		if(HLI_CAN_Read16(DataTable[NodeIDReg], COMM_REG_DEV_STATE, &Register))
			*StateStorage = Register;
		else
			Result = FALSE;
	}

	return Result;
}
//-----------------------------
