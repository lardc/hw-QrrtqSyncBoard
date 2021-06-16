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
	ZwGPIO_PinToInput(PIN_DCU1_READY, TRUE, GPIO_NSAMPLE);
	ZwGPIO_PinToInput(PIN_DCU2_READY, TRUE, GPIO_NSAMPLE);
	ZwGPIO_PinToInput(PIN_DCU3_READY, TRUE, GPIO_NSAMPLE);
	ZwGPIO_PinToInput(PIN_SAFETY, TRUE, GPIO_NSAMPLE);
	ZwGPIO_PinToInput(PIN_PRESSURE, TRUE, GPIO_NSAMPLE);

	// Reset to default state
	ZwGPIO_WritePin(PIN_CSU_PS, TRUE);
	ZwGPIO_WritePin(PIN_SWITCH_DUT, FALSE);
	ZwGPIO_WritePin(PIN_CONTROL_DUT, FALSE);
	ZwGPIO_WritePin(PIN_LED, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_RCU, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_DCU, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_FCROVU, FALSE);
	ZwGPIO_WritePin(PIN_CSU_FAN, FALSE);
	ZwGPIO_WritePin(PIN_CSU_SYNC, FALSE);
	ZwGPIO_WritePin(PIN_CSU_DISCH, FALSE);
	ZwGPIO_WritePin(PIN_SYNC_SCOPE, FALSE);
	ZwGPIO_WritePin(PIN_PC_PWR, FALSE);
	ZwGPIO_WritePin(PIN_DBG, FALSE);
	ZwGPIO_WritePin(PIN_SAFETY_EN, FALSE);
	ZwGPIO_WritePin(PIN_GATE_RELAY, FALSE);
	ZwGPIO_WritePin(PIN_SB_EN, FALSE);
	ZwGPIO_WritePin(PIN_QCUHC_CTRL, FALSE);

   	// Configure pins
	ZwGPIO_PinToOutput(PIN_CSU_PS);
	ZwGPIO_PinToOutput(PIN_SWITCH_DUT);
	ZwGPIO_PinToOutput(PIN_CONTROL_DUT);
	ZwGPIO_PinToOutput(PIN_LED);
	ZwGPIO_PinToOutput(PIN_SYNC_RCU);
	ZwGPIO_PinToOutput(PIN_SYNC_DCU);
	ZwGPIO_PinToOutput(PIN_SYNC_FCROVU);
	ZwGPIO_PinToOutput(PIN_CSU_FAN);
	ZwGPIO_PinToOutput(PIN_CSU_SYNC);
	ZwGPIO_PinToOutput(PIN_CSU_DISCH);
	ZwGPIO_PinToOutput(PIN_SYNC_SCOPE);
	ZwGPIO_PinToOutput(PIN_PC_PWR);
	ZwGPIO_PinToOutput(PIN_DBG);
	ZwGPIO_PinToOutput(PIN_SAFETY_EN);
	ZwGPIO_PinToOutput(PIN_GATE_RELAY);
	ZwGPIO_PinToOutput(PIN_SB_EN);
	ZwGPIO_PinToOutput(PIN_QCUHC_CTRL);

	// Configure external interrupt
	ZwGPIO_PinToInput(PIN_CSU_ITRIG, TRUE, GPIO_NSAMPLE);
	ZwXINT1_Init(PIN_CSU_ITRIG, TRIG_RISE_EDGE);
}
// ----------------------------------------

Boolean ZbGPIO_DCU1_Ready()
{
	return ZwGPIO_ReadPin(PIN_DCU1_READY);
}
// ----------------------------------------

Boolean ZbGPIO_DCU2_Ready()
{
	return ZwGPIO_ReadPin(PIN_DCU2_READY);
}
// ----------------------------------------

Boolean ZbGPIO_DCU3_Ready()
{
	return ZwGPIO_ReadPin(PIN_DCU3_READY);
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
	ZwGPIO_WritePin(PIN_SYNC_FCROVU, Set);
}
// ----------------------------------------

void ZbGPIO_DUT_Switch(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SWITCH_DUT, Set);
}
// ----------------------------------------

void ZbGPIO_DUT_ControlEnable(Boolean Set)
{
	ZwGPIO_WritePin(PIN_GATE_RELAY, Set);
}
// ----------------------------------------

void ZbGPIO_DUT_Control(Boolean Set)
{
	ZwGPIO_WritePin(PIN_CONTROL_DUT, Set);
}
// ----------------------------------------

void ZbGPIO_PC_TurnOn(Boolean Set)
{
	ZwGPIO_WritePin(PIN_PC_PWR, Set);
}
// ----------------------------------------

void ZbGPIO_LED_Toggle()
{
	ZwGPIO_TogglePin(PIN_LED);
}
// ----------------------------------------

void ZbGPIO_SafetyEnable(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SAFETY_EN, Set);
}
// ----------------------------------------

Boolean ZbGPIO_SafetyCheck()
{
	return !ZwGPIO_ReadPin(PIN_SAFETY);
}
// ----------------------------------------

Boolean ZbGPIO_PressureCheck()
{
	return !ZwGPIO_ReadPin(PIN_PRESSURE);
}
// ----------------------------------------

void ZbGPIO_SensingBoardEnable(Boolean Set)
{
	ZwGPIO_WritePin(PIN_SB_EN, Set);
}
// ----------------------------------------

void ZbGPIO_QCUHCEnable(Boolean Set)
{
	ZwGPIO_WritePin(PIN_QCUHC_CTRL, Set);
}
// ----------------------------------------

// No more.
