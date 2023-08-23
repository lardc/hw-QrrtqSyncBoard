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
	ZwGPIO_PinToInput(PIN_DCU_READY, TRUE, GPIO_NSAMPLE);
	ZwGPIO_PinToInput(PIN_CSU_ITRIG, TRUE, GPIO_NSAMPLE);

	// Reset to default state
	ZwGPIO_WritePin(PIN_CSU_PS, TRUE);
	ZwGPIO_WritePin(PIN_SWITCH_DUT, FALSE);
	ZwGPIO_WritePin(PIN_CONTROL_DUT, FALSE);
	ZwGPIO_WritePin(PIN_LED, FALSE);
	ZwGPIO_WritePin(PIN_LED2, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_RCU, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_DCU, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_FCROVU, FALSE);
	ZwGPIO_WritePin(PIN_CSU_FAN, FALSE);
	ZwGPIO_WritePin(PIN_CSU_SYNC, FALSE);
	ZwGPIO_WritePin(PIN_CSU_DISCH, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_SCOPE, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_SCOPE2, FALSE);
	ZwGPIO_WritePin(PIN_PC_PWR, FALSE);
	ZwGPIO_WritePin(PIN_DBG, FALSE);

   	// Configure pins
	ZwGPIO_PinToOutput(PIN_CSU_PS);
	ZwGPIO_PinToOutput(PIN_SWITCH_DUT);
	ZwGPIO_PinToOutput(PIN_CONTROL_DUT);
	ZwGPIO_PinToOutput(PIN_LED);
	ZwGPIO_PinToOutput(PIN_LED2);
	ZwGPIO_PinToOutput(PIN_SYNC_RCU);
	ZwGPIO_PinToOutput(PIN_SYNC_DCU);
	ZwGPIO_PinToOutput(PIN_SYNC_FCROVU);
	ZwGPIO_PinToOutput(PIN_CSU_FAN);
	ZwGPIO_PinToOutput(PIN_CSU_SYNC);
	ZwGPIO_PinToOutput(PIN_CSU_DISCH);
	ZwGPIO_PinToOutput(PIN_SYNC_SCOPE);
	ZwGPIO_PinToOutput(PIN_SYNC_SCOPE2);
	ZwGPIO_PinToOutput(PIN_PC_PWR);
	ZwGPIO_PinToOutput(PIN_DBG);
}
// ----------------------------------------

Boolean ZbGPIO_CSU_Itrig()
{
	return ZwGPIO_ReadPin(PIN_CSU_ITRIG);
}
// ----------------------------------------

Boolean ZbGPIO_DCU_Ready()
{
	return ZwGPIO_ReadPin(PIN_DCU_READY);
}
// ----------------------------------------

void ZbGPIO_CSU_PWRCtrl(Boolean Set)
{
	ZwGPIO_WritePin(PIN_CSU_PS, !Set);
}
// ----------------------------------------

void ZbGPIO_CSU_Disch(Boolean Set)
{
	ZwGPIO_WritePin(PIN_CSU_DISCH, Set);
}
// ----------------------------------------

void ZbGPIO_CSU_FAN(Boolean Set)
{
	ZwGPIO_WritePin(PIN_CSU_FAN, Set);
}
// ----------------------------------------

void ZbGPIO_CSU_Sync(Boolean Set)
{
	ZwGPIO_WritePin(PIN_CSU_SYNC, Set);
}
// ----------------------------------------

void ZbGPIO_SCOPE_Sync(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SYNC_SCOPE, Set);
	ZwGPIO_WritePin(PIN_SYNC_SCOPE2, Set);
}
// ----------------------------------------

void ZbGPIO_RCU_Sync(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SYNC_RCU, Set);
}
// ----------------------------------------

void ZbGPIO_DCU_Sync(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SYNC_DCU, Set);
}
// ----------------------------------------

void ZbGPIO_FCROVU_Sync(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SYNC_FCROVU, !Set);
}
// ----------------------------------------

void ZbGPIO_DUT_Switch(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SWITCH_DUT, Set);
}
// ----------------------------------------

void ZbGPIO_DUT_ControlEnable(Boolean Set)
{
}
// ----------------------------------------

void ZbGPIO_DUT_Control(Boolean Set)
{
	ZwGPIO_WritePin(PIN_CONTROL_DUT, Set);
}
// ----------------------------------------

void ZbGPIO_PC_TurnOn()
{
	ZwGPIO_WritePin(PIN_PC_PWR, TRUE);
	DELAY_US(500000);
	ZwGPIO_WritePin(PIN_PC_PWR, FALSE);
}
// ----------------------------------------

void ZbGPIO_LED_Toggle()
{
	ZwGPIO_TogglePin(PIN_LED);
}
// ----------------------------------------

void ZbGPIO_SafetyEnable(Boolean Set)
{
}
// ----------------------------------------

Boolean ZbGPIO_SafetyCheck()
{
	return FALSE;
}
// ----------------------------------------

Boolean ZbGPIO_PressureCheck()
{
	return TRUE;
}
// ----------------------------------------

void ZbGPIO_SensingBoardEnable(Boolean Set)
{
}
// ----------------------------------------

void ZbGPIO_QCUHCEnable(Boolean Set)
{
}
// ----------------------------------------

// No more.
