// -----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

// Header
#include "ZbGPIO.h"
#include "SysConfig.h"

// Functions
//
void ZbGPIO_Init()
{
	// Configure input pin
	ZwGPIO_PinToInput(PIN_DC_READY, TRUE, GPIO_NSAMPLE);

	// Reset to default state
	ZwGPIO_WritePin(PIN_SWITCH_HV_IGBT, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_SCOPE, FALSE);
	ZwGPIO_WritePin(PIN_SWITCH_REV_CUR, FALSE);
	ZwGPIO_WritePin(PIN_SWITCH_DIR_CUR, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_FCROVU, TRUE);
	ZwGPIO_WritePin(PIN_SWITCH_DUT, FALSE);
	ZwGPIO_WritePin(PIN_LED_1, FALSE);
	ZwGPIO_WritePin(PIN_LED_2, FALSE);
   	// Configure pins
   	ZwGPIO_PinToOutput(PIN_SWITCH_HV_IGBT);
   	ZwGPIO_PinToOutput(PIN_SYNC_SCOPE);
   	ZwGPIO_PinToOutput(PIN_SWITCH_REV_CUR);
   	ZwGPIO_PinToOutput(PIN_SWITCH_DIR_CUR);
   	ZwGPIO_PinToOutput(PIN_SYNC_FCROVU);
   	ZwGPIO_PinToOutput(PIN_SWITCH_DUT);
   	ZwGPIO_PinToOutput(PIN_LED_1);
   	ZwGPIO_PinToOutput(PIN_LED_2);
}
// ----------------------------------------

Boolean ZbGPIO_DirectCurrentReady()
{
	return !ZwGPIO_ReadPin(PIN_DC_READY);
}
// ----------------------------------------

void ZbGPIO_SwitchHVIGBT(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SWITCH_HV_IGBT, Set);
}
// ----------------------------------------

void ZbGPIO_SyncSCOPE(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SYNC_SCOPE, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchReverseCurrent(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SWITCH_REV_CUR, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchDirectCurrent(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SWITCH_DIR_CUR, Set);
}
// ----------------------------------------

void ZbGPIO_SyncFCROVU(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SYNC_FCROVU, !Set);
}
// ----------------------------------------

void ZbGPIO_SwitchDUT(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SWITCH_DUT, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchLED1(Boolean Set)
{
	ZwGPIO_WritePin(PIN_LED_1, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchLED2(Boolean Set)
{
	ZwGPIO_WritePin(PIN_LED_2, Set);
}
// ----------------------------------------

void ZbGPIO_ToggleLED1()
{
	ZwGPIO_TogglePin(PIN_LED_1);
}
// ----------------------------------------

// No more.
