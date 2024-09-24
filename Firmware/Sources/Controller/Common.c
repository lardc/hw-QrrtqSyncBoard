// Header
#include "Common.h"

// Includes
#include "HighLevelInterface.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"

// Forward functions
void CMN_WaitNodesReadyX(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState, Boolean NodesConfig, Int16U Fault, LogicState NewState);

// Functions
//
Boolean CMN_UpdateNodeState(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity)
{
	if(!DevEntity->Emulate)
	{
		Int16U Register;
		if(HLI_CAN_Read16(DataTable[NodeIDReg], COMM_REG_DEV_STATE, &Register))
			DevEntity->State = Register;
		else
			return FALSE;
	}
	
	return TRUE;
}
//-----------------------------

void CMN_ResetNodeFault(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState)
{
	if(!DevEntity->Emulate && DevEntity->State == CDS_Fault)
	{
		if(HLI_CAN_CallAction(DataTable[NodeIDReg], COMM_ACT_FAULT_CLEAR))
			*CurrentLogicState = NextLogicState;
	}
	else
		*CurrentLogicState = NextLogicState;
}
//-----------------------------

void CMN_NodePowerOn(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState)
{
	if(!DevEntity->Emulate)
	{
		switch(DevEntity->State)
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
		*CurrentLogicState = NextLogicState;
}
//-----------------------------

void CMN_NodePowerOff(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState)
{
	if(!DevEntity->Emulate)
	{
		switch(DevEntity->State)
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
		*CurrentLogicState = NextLogicState;
}
//-----------------------------

void CMN_ConfigDRCU(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, pDRCUConfig Config,
		volatile LogicState *CurrentLogicState, LogicState NextLogicState)
{
	if(!DevEntity->Emulate)
	{
		if(HLI_CAN_Write16(DataTable[NodeIDReg], DRCU_REG_I_MAX_VALUE, Config->Current))
			if(HLI_CAN_Write16(DataTable[NodeIDReg], DRCU_REG_I_RATE, Config->CurrentRate))
				if(HLI_CAN_Write16(DataTable[NodeIDReg], REG_I_TO_V_INTPS_EXT_OFFSET, Config->V_Offset))
					if(HLI_CAN_Write16(DataTable[NodeIDReg], REG_I_TO_V_INTPS_EXT_K, Config->V_K))
						if(HLI_CAN_Write16(DataTable[NodeIDReg], REG_I_TO_V_INTPS_EXT_K2, Config->V_K2))
							if(HLI_CAN_Write16(DataTable[NodeIDReg], REG_CTRL1_EXT_OFFSET, Config->I_Offset))
								if(HLI_CAN_Write16(DataTable[NodeIDReg], REG_CTRL1_EXT_K, Config->I_K))
									if(HLI_CAN_CallAction(DataTable[NodeIDReg], DCRU_ACT_CONFIG))
										*CurrentLogicState = NextLogicState;
	}
	else
		*CurrentLogicState = NextLogicState;
}
//-----------------------------

void CMN_WaitNodesReadyPowerOn(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState)
{
	CMN_WaitNodesReadyX(TimeCounter, Timeout, FullStateStorage, CurrentLogicState, FALSE, FAULTEX_PON_TIMEOUT, LS_None);
}
//-----------------------------

void CMN_WaitNodesReadyPreConfig(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState, LogicState NewState)
{
	CMN_WaitNodesReadyX(TimeCounter, Timeout, FullStateStorage, CurrentLogicState, FALSE, FAULTEX_PRECFG_TIMEOUT, NewState);
}
//-----------------------------

void CMN_WaitNodesReadyConfig(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState)
{
	CMN_WaitNodesReadyX(TimeCounter, Timeout, FullStateStorage, CurrentLogicState, TRUE, FAULTEX_CFG_TIMEOUT, LS_None);
}
//-----------------------------

void CMN_WaitNodesReadyX(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState, Boolean NodesConfig, Int16U Fault, LogicState NewState)
{
	Int16U DRCUWaitState, ScopeWaitState, FCROVUWaitState;

	if(NodesConfig)
	{
		FCROVUWaitState = CDS_ConfigReady;
		DRCUWaitState = DRCU_DS_ConfigReady;
		ScopeWaitState = CDS_InProcess;
	}
	else
	{
		FCROVUWaitState = CDS_Ready;
		DRCUWaitState = CDS_Ready;
		ScopeWaitState = CDS_None;
	}

	Boolean ReadyCROVU	= FullStateStorage->CROVU.Emulate || (FullStateStorage->CROVU.State == CDS_Ready);
	Boolean ReadyFCROVU	= FullStateStorage->FCROVU.Emulate || (FullStateStorage->FCROVU.State == FCROVUWaitState);
	Boolean ReadyDCU1	= FullStateStorage->DCU1.Emulate || (FullStateStorage->DCU1.State == DRCUWaitState);
	Boolean ReadyDCU2	= FullStateStorage->DCU2.Emulate || (FullStateStorage->DCU2.State == DRCUWaitState);
	Boolean ReadyDCU3	= FullStateStorage->DCU3.Emulate || (FullStateStorage->DCU3.State == DRCUWaitState);
	Boolean ReadyRCU1	= FullStateStorage->RCU1.Emulate || (FullStateStorage->RCU1.State == DRCUWaitState);
	Boolean ReadyRCU2	= FullStateStorage->RCU2.Emulate || (FullStateStorage->RCU2.State == DRCUWaitState);
	Boolean ReadyRCU3	= FullStateStorage->RCU3.Emulate || (FullStateStorage->RCU3.State == DRCUWaitState);
	Boolean ReadySCOPE	= FullStateStorage->SCOPE.Emulate || (FullStateStorage->SCOPE.State == ScopeWaitState);

	if(*CurrentLogicState != NewState && *CurrentLogicState != LS_Error)
	{
		if(Timeout > TimeCounter)
		{
			if(ReadyCROVU && ReadyFCROVU && ReadySCOPE &&
					ReadyDCU1 && ReadyDCU2 && ReadyDCU3 &&
					ReadyRCU1 && ReadyRCU2 && ReadyRCU3)
			{
				*CurrentLogicState = NewState;
			}
		}
		else
		{
			if(!ReadyCROVU)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, Fault);
			}
			else if(!ReadyFCROVU)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, Fault);
			}
			else if(!ReadyDCU1)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, Fault);
			}
			else if(!ReadyDCU2)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, Fault);
			}
			else if(!ReadyDCU3)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, Fault);
			}
			else if(!ReadyRCU1)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, Fault);
			}
			else if(!ReadyRCU2)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, Fault);
			}
			else if(!ReadyRCU3)
			{
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, Fault);
			}
			else if(!ReadySCOPE)
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
