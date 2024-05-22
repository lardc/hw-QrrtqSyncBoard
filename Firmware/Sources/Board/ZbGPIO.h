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
Boolean ZbGPIO_DirectCurrentReady();
//
void ZbGPIO_SwitchHVIGBT(Boolean Set);
void ZbGPIO_SyncSCOPE(Boolean Set);
void ZbGPIO_SwitchReverseCurrent(Boolean Set);
void ZbGPIO_SwitchDirectCurrent(Boolean Set);
void ZbGPIO_SyncFCROVU(Boolean Set);
void ZbGPIO_SwitchDUT(Boolean Set);
void ZbGPIO_SwitchLED1(Boolean Set);
void ZbGPIO_SwitchLED2(Boolean Set);
void ZbGPIO_ToggleLED1();

#endif // __ZBGPIO_H
