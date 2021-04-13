#ifndef __COMMON_H
#define __COMMON_H

// Include
#include "stdinc.h"
#include "CommonDictionary.h"
#include "Logic.h"

// Functions
Boolean CMN_UpdateNodeState(Boolean Emulate, Int16U NodeIDReg, volatile Int16U *StateStorage);
void CMN_ResetNodeFault(Boolean Emulate, Int16U NodeIDReg, Int16U NodeState,
		volatile LogicState *CurrentState, LogicState NextState);

#endif // __COMMON_H
