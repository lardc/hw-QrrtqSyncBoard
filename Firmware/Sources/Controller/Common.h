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
	Int16U CurrentRate;
	Int32U RCUTrigOffsetTicks;
} DRCUConfig, *pDRCUConfig;

// Functions
Boolean CMN_UpdateNodeState(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity);
void CMN_ResetNodeFault(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState);
void CMN_NodePowerOn(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_NodePowerOff(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_ConfigDRCU(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, pDRCUConfig Config,
		volatile LogicState *CurrentLogicState, LogicState NextLogicState);
void CMN_WaitNodesReadyPreConfig(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState, LogicState NewState);
void CMN_WaitNodesReadyConfig(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState);
void CMN_WaitNodesReadyPowerOn(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState);

#endif // __COMMON_H
