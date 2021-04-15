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
	Int16U RiseRate;
	Int16U FallRate;
} DRCUConfig, *pDRCUConfig;

// Functions
Boolean CMN_UpdateNodeState(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage);
void CMN_ResetNodeFault(Boolean Emulate, Int16U NodeIDReg, Int16U StateStorage, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState);
void CMN_NodePowerOn(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_NodePowerOff(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage,
		volatile LogicState *CurrentLogicState, Int16U FaultCode, LogicState NextLogicState);
void CMN_ConfigDRCU(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage, pDRCUConfig Config,
		volatile LogicState *CurrentLogicState, LogicState NextLogicState);
void CMN_WaitNodesReady(Int64U TimeCounter, Int64U Timeout, ExternalDeviceState FullStateStorage,
		volatile LogicState *CurrentLogicState);
Boolean CMN_ReadDRCUCurrent(Boolean Emulate, Int16U NodeIDReg, Int16U StateStorage, pInt16U Current);

#endif // __COMMON_H
