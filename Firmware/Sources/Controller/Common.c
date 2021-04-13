// Header
#include "Common.h"

// Includes
#include "HighLevelInterface.h"
#include "DataTable.h"

// Functions
//
Boolean CMN_UpdateNodeState(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage)
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

void CMN_ResetNodeFault(Boolean Emulate, Int16U NodeIDReg, Int16U NodeState,
		volatile LogicState *CurrentState, LogicState NextState)
{
	if(!Emulate && NodeState == CDS_Fault)
	{
		if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_FAULT_CLEAR))
			*CurrentState = NextState;
	}
	else
		*CurrentState = NextState;
}
//-----------------------------
