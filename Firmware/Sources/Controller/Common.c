// Header
#include "Common.h"

// Includes
#include "HighLevelInterface.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"

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

void CMN_ResetNodeFault(Boolean Emulate, Int16U NodeIDReg, Int16U StateStorage, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState)
{
	if(!Emulate && StateStorage == CDS_Fault)
	{
		if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_FAULT_CLEAR))
			*CurrentLogicState = NextLogicState;
	}
	else
		*CurrentLogicState = NextLogicState;
}
//-----------------------------

void CMN_NodePowerOn(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState)
{
	if(!Emulate)
	{
		switch(*StateStorage)
		{
			case CDS_None:
				if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_ENABLE_POWER))
					*CurrentLogicState = NextLogicState;
				break;

			case CDS_Ready:
				if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_DISABLE_POWER))
					if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_ENABLE_POWER))
						*CurrentLogicState = NextLogicState;
				break;

			case CDS_Fault:
				if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_FAULT_CLEAR))
					if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_ENABLE_POWER))
						*CurrentLogicState = NextLogicState;
				break;

			case CDS_Disabled:
				*CurrentLogicState = LS_Error;
				CONTROL_SwitchToFault(FaultCode, FAULTEX_PON_WRONG_STATE);
				break;
		}
	}
	else
	{
		*StateStorage = CDS_Ready;
		*CurrentLogicState = NextLogicState;
	}
}
//-----------------------------

void CMN_ConfigDRCU(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage, pDRCUConfig Config,
		volatile LogicState *CurrentLogicState, LogicState NextLogicState)
{
	if(!Emulate)
	{
		if(HLI_CAN_Write16(DataTable[NodeIDReg], DRCU_REG_I_MAX_VALUE, Config->Current))
			if(HLI_CAN_Write16(DataTable[NodeIDReg], DRCU_REG_I_RATE_RISE, Config->RiseRate))
				if(HLI_CAN_Write16(DataTable[NodeIDReg], DRCU_REG_I_RATE_FALL, Config->FallRate))
					if(HLI_CAN_CallAction(DataTable[NodeIDReg], DCRU_ACT_CONFIG))
						*CurrentLogicState = NextLogicState;
	}
	else
	{
		*StateStorage = CDS_Ready;
		*CurrentLogicState = NextLogicState;
	}
}
//-----------------------------

void CMN_WaitNodesReady(Int64U TimeCounter, Int64U Timeout, ExternalDeviceState FullStateStorage,
		volatile LogicState *CurrentLogicState)
{
	if(*CurrentLogicState != LS_None || *CurrentLogicState != LS_Error)
	{
		if(Timeout > TimeCounter)
		{
			if(FullStateStorage.DS_CROVU == CDS_Ready
					&& FullStateStorage.DS_FCROVU == CDS_Ready
					&& FullStateStorage.DS_DCU1 == CDS_Ready
					&& FullStateStorage.DS_DCU2 == CDS_Ready
					&& FullStateStorage.DS_DCU3 == CDS_Ready
					&& FullStateStorage.DS_RCU1 == CDS_Ready
					&& FullStateStorage.DS_RCU2 == CDS_Ready
					&& FullStateStorage.DS_RCU3 == CDS_Ready
					&& FullStateStorage.DS_SCOPE == CDS_None)
			{
				*CurrentLogicState = LS_None;
			}
		}
		else
		{
			if(FullStateStorage.DS_CROVU != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_FCROVU != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_DCU1 != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_DCU2 != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_DCU3 != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_RCU1 != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_RCU2 != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_RCU3 != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, FAULTEX_PON_TIMEOUT);
			}
			else if(FullStateStorage.DS_SCOPE != CDS_None)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_PON_TIMEOUT);
			}
			else
				CONTROL_SwitchToFault(FAULT_LOGIC_GENERAL, FAULTEX_PON_TIMEOUT);

			*CurrentLogicState = LS_Error;
		}
	}
}
//-----------------------------
