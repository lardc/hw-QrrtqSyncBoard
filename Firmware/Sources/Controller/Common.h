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
	Int16U CurrentRateNum;
	Int32U RCUTrigOffsetTicks;
	Int16S V_Offset;
	Int16S V_K;
	Int16S V_K2;
	Int16S I_Ctrl_Offset;
	Int16S I_Ctrl_K;
	Int16S I_P0;
	Int16S I_P1;
	Int16S I_P2;

} DRCUConfig, *pDRCUConfig;

typedef struct __ScopeConfig
{
	Int16U ScopeCurrentScaleResult;
	Int16U ScopeCurrentSamplingTime;

} ScopeConfig, *pScopeConfig;

// Functions
Boolean CMN_UpdateNodeState(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity);
void CMN_ResetNodeFault(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState);
void CMN_NodePowerOn(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_NodePowerOff(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_ConfigDRCU(Int16U NodeIDReg, volatile DeviceStateEntity *DevEntity, pDRCUConfig Config,
		volatile LogicState *CurrentLogicState, LogicState NextLogicState,
		Int64U TimeCounter, Int64U Timeout, Int16U Fault);
void CMN_WaitNodesReadyPreConfig(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState, LogicState NewState);
void CMN_WaitNodesReadyConfig(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState, Boolean MuteFCROVU);
void CMN_WaitNodesReadyPowerOn(Int64U TimeCounter, Int64U Timeout, volatile ExternalDeviceState *FullStateStorage,
		volatile LogicState *CurrentLogicState);

#endif // __COMMON_H
