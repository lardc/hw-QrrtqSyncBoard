#ifndef __COMMON_H
#define __COMMON_H

// Include
#include "stdinc.h"
#include "CommonDictionary.h"
#include "DRCUDictionary.h"
#include "Logic.h"

// Types
typedef struct __DRCUConfig
{
	Int16U Current;
	Int16U CurrentRate_x100;
	Int32U RCUTrigOffsetTicks;
} DRCUConfig, *pDRCUConfig;

// Functions
Boolean CMN_UpdateNodeState(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity);
void CMN_ResetNodeFault(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState);
void CMN_NodePowerOn(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_NodePowerOff(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_ConfigDRCU(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage, pDRCUConfig Config,
		volatile LogicState *CurrentLogicState, LogicState NextLogicState);
void CMN_WaitNodesReady(Int64U TimeCounter, Int64U Timeout, ExternalDeviceState FullStateStorage,
		volatile LogicState *CurrentLogicState, Boolean NodesConfig);
Boolean CMN_ReadDRCUCurrent(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage, pInt16U Current);

#endif // __COMMON_H
