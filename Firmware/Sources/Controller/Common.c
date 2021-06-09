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

void CMN_NodePowerOff(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState)
{
	if(!Emulate)
	{
		switch(*StateStorage)
		{
			case CDS_None:
				break;

			case CDS_Ready:
				if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_DISABLE_POWER))
					*CurrentLogicState = NextLogicState;
				break;

			default:
				*CurrentLogicState = LS_Error;
				CONTROL_SwitchToFault(FaultCode, FAULTEX_POFF_WRONG_STATE);
				break;
		}
	}
	else
	{
		*StateStorage = CDS_None;
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
			if(HLI_CAN_Write16(DataTable[NodeIDReg], DRCU_REG_I_RATE, Config->CurrentRate_x100))
				if(HLI_CAN_CallAction(DataTable[NodeIDReg], DCRU_ACT_CONFIG))
					*CurrentLogicState = NextLogicState;
	}
	else
	{
		*StateStorage = DRCU_DS_ConfigReady;
		*CurrentLogicState = NextLogicState;
	}
}
//-----------------------------

void CMN_WaitNodesReady(Int64U TimeCounter, Int64U Timeout, ExternalDeviceState FullStateStorage,
		volatile LogicState *CurrentLogicState, Boolean NodesConfig)
{
	Int16U DRCUWaitState, Fault;

	if(NodesConfig)
	{
		DRCUWaitState = DRCU_DS_ConfigReady;
		Fault = FAULTEX_CFG_TIMEOUT;
	}
	else
	{
		DRCUWaitState = CDS_Ready;
		Fault = FAULTEX_PON_TIMEOUT;
	}

	if(*CurrentLogicState != LS_None && *CurrentLogicState != LS_Error)
	{
		if(Timeout > TimeCounter)
		{
			if(FullStateStorage.DS_CROVU == CDS_Ready
					&& FullStateStorage.DS_FCROVU == CDS_Ready
					&& FullStateStorage.DS_DCU1 == DRCUWaitState
					&& FullStateStorage.DS_DCU2 == DRCUWaitState
					&& FullStateStorage.DS_DCU3 == DRCUWaitState
					&& FullStateStorage.DS_RCU1 == DRCUWaitState
					&& FullStateStorage.DS_RCU2 == DRCUWaitState
					&& FullStateStorage.DS_RCU3 == DRCUWaitState
					&& FullStateStorage.DS_SCOPE == CDS_None)
			{
				*CurrentLogicState = LS_None;
			}
		}
		else
		{
			if(FullStateStorage.DS_CROVU != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, Fault);
			}
			else if(FullStateStorage.DS_FCROVU != CDS_Ready)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, Fault);
			}
			else if(FullStateStorage.DS_DCU1 != DRCUWaitState)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, Fault);
			}
			else if(FullStateStorage.DS_DCU2 != DRCUWaitState)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, Fault);
			}
			else if(FullStateStorage.DS_DCU3 != DRCUWaitState)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, Fault);
			}
			else if(FullStateStorage.DS_RCU1 != DRCUWaitState)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, Fault);
			}
			else if(FullStateStorage.DS_RCU2 != DRCUWaitState)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, Fault);
			}
			else if(FullStateStorage.DS_RCU3 != DRCUWaitState)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, Fault);
			}
			else if(FullStateStorage.DS_SCOPE != CDS_None)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, Fault);
			}
			else
				CONTROL_SwitchToFault(FAULT_LOGIC_GENERAL, Fault);

			*CurrentLogicState = LS_Error;
		}
	}
}
//-----------------------------

Boolean CMN_ReadDRCUCurrent(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage, pInt16U Current)
{
	if(!Emulate)
	{
		if(*StateStorage == DRCU_DS_InProcess || *StateStorage == CDS_Ready)
			if(HLI_CAN_Read16(DataTable[NodeIDReg], DRCU_REG_CURRENT, Current))
				return TRUE;
	}
	else
	{
		*Current = 0;
		*StateStorage = CDS_Ready;

		return TRUE;
	}

	return FALSE;
}
//-----------------------------
