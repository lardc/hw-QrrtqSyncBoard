#ifndef __COMMON_H
#define __COMMON_H

// Include
#include "stdinc.h"
#include "CommonDictionary.h"
#include "Logic.h"

// Functions
Boolean CMN_UpdateNodeState(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage);
void CMN_ResetNodeFault(Boolean Emulate, Int16U NodeIDReg, Int16U StateStorage, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState);
void CMN_NodePowerOn(Boolean Emulate, Int16U NodeIDReg, pInt16U StateStorage, volatile LogicState *CurrentLogicState,
		LogicState NextLogicState, Int16U FaultCode);

#endif // __COMMON_H
