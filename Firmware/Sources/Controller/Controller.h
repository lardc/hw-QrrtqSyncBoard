// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "stdinc.h"
//
#include "ZwDSP.h"
#include "Global.h"


// Types
//
typedef enum __DeviceState
{
	DS_None			= 0,
	DS_Fault		= 1,
	DS_Disabled		= 2,
	DS_PowerOn		= 3,
	DS_Ready		= 4,
	DS_InProcess	= 5
} DeviceState;
//
typedef void (*FUNC_AsyncDelegate)();


// Variables
//
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
//
extern Int16U CONTROL_Values_1[VALUES_x_SIZE];
extern Int16U CONTROL_Values_2[VALUES_x_SIZE];
extern Int16U CONTROL_Values_Slave[VALUES_x_SIZE];
extern volatile Int16U CONTROL_Values_1_Counter, CONTROL_Values_2_Counter, CONTROL_Values_Slave_Counter;
extern volatile Int16U CONTROL_BootLoaderRequest;
//
extern Int16U CONTROL_ValDiag1[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag2[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag3[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag4[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag5[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag6[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag7[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag8[UNIT_MAX_NUM_OF_PULSES];
extern Int16U CONTROL_ValDiag9[UNIT_MAX_NUM_OF_PULSES];
extern volatile Int16U CONTROL_ValDiag_Counter;


// Functions
//
// Initialize controller
void CONTROL_Init(Boolean BadClockDetected);
// Update low-priority states
void CONTROL_Idle();
// Main control cycle
void CONTROL_Update();
// Notify that CAN system fault occurs
void CONTROL_NotifyCANbFault(ZwCAN_SysFlags Flag);
// Switch device to fault state
void CONTROL_SwitchToFault(Int16U FaultReason, Int16U FaultReasonExt);
// Delegate async action
void CONTROL_RequestDPC(FUNC_AsyncDelegate Action);


#endif // __CONTROLLER_H
