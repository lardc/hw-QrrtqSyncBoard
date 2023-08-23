// -----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

#ifndef __ZBGPIO_H
#define __ZBGPIO_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"

// Functions
//
void ZbGPIO_Init();
//
Boolean ZbGPIO_DCU_Ready();
Boolean ZbGPIO_CSU_Itrig();
//
void ZbGPIO_CSU_PWRCtrl(Boolean Set);
void ZbGPIO_CSU_Disch(Boolean Set);
void ZbGPIO_CSU_FAN(Boolean Set);
void ZbGPIO_CSU_Sync(Boolean Set);
void ZbGPIO_SCOPE_Sync(Boolean Set);
void ZbGPIO_RCU_Sync(Boolean Set);
void ZbGPIO_DCU_Sync(Boolean Set);
void ZbGPIO_FCROVU_Sync(Boolean Set);
void ZbGPIO_DUT_Switch(Boolean Set);
void ZbGPIO_DUT_Control(Boolean Set);
void ZbGPIO_PC_TurnOn();
void ZbGPIO_LED_Toggle();
void ZbGPIO_SafetyEnable(Boolean Set);
Boolean ZbGPIO_SafetyCheck();
Boolean ZbGPIO_PressureCheck();
void ZbGPIO_DUT_ControlEnable(Boolean Set);
void ZbGPIO_SensingBoardEnable(Boolean Set);
void ZbGPIO_QCUHCEnable(Boolean Set);

#endif // __ZBGPIO_H
