// -----------------------------------------
// Main logic library
// ----------------------------------------

// Header
#include "Logic.h"
//
// Includes
#include "xCCICommon.h"
#include "Common.h"
#include "HighLevelInterface.h"
#include "Global.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"
#include "ZbBoard.h"

// Definitions
//
typedef struct __ExternalDeviceState
{
	Int16U DS_CROVU;
	Int16U DS_FCROVU;
	Int16U DS_DCU1;
	Int16U DS_DCU2;
	Int16U DS_DCU3;
	Int16U DS_RCU1;
	Int16U DS_RCU2;
	Int16U DS_RCU3;
	Int16U DS_CSU;
	Int16U DS_SCOPE;
} ExternalDeviceState;

// Variables
//
volatile DeviceSubState LOGIC_StateRealTime = LSRT_None;
volatile Int32U LOGIC_RealTimeCounter = 0;
static volatile Int64U Timeout;
static volatile Int64U CSU_FanTimeout;
static volatile LogicState LOGIC_State = LS_None;
static volatile ExternalDeviceState LOGIC_ExtDeviceState;
//
static MeasurementResult Results[UNIT_MAX_NUM_OF_PULSES];
static volatile Int16U ResultsCounter, MeasurementMode;
//
static Boolean EmulateCROVU, EmulateFCROVU, EmulateDCU1, EmulateDCU2, EmulateDCU3, EmulateRCU1, EmulateRCU2,
		EmulateRCU3, EmulateSCOPE, MuteCROVU, EmulateCSU;
static Boolean CacheUpdate = FALSE, CacheSinglePulse = FALSE, DCPulseFormed = FALSE;
static volatile Boolean TqFastThyristor = FALSE, DUTFinalIncrease = FALSE;
static Int16U DC_Current, DC_CurrentRiseRate, DC_CurrentFallRate, DC_CurrentPlateTicks, DC_CurrentZeroPoint, RC_Current,
		RC_CurrentFallRate, RC_CurrentFallRate;
static Int16U DC_CurrentMax, RC_CurrentMax;
static Int16U UnitsStartTimeOut = 0;
static Int16U CROVU_Voltage, CROVU_VoltageRate, FCROVU_IShortCircuit;
static volatile Int16U CROVU_TrigTime, CROVU_TrigTime_LastHalf;
static volatile Int16U LOGIC_PulseNumRemain, LOGIC_DCReadyRetries, LOGIC_OperationResult, LOGIC_DriverOffTicks;
static Int16U CSUVoltage = 0;
//

// Forward functions
//
void LOGIC_PreciseEventInit(Int16U usTime);
void LOGIC_PreciseEventStart();
void LOGIC_TqExtraLogic(Boolean DeviceTriggered);

// Functions
//
void LOGIC_RealTime()
{
	static Int32U TimeReverseStart = 0, TimeReverseStop = 0, TimeBeforeDataRead = 0;
	
	if(LOGIC_StateRealTime != LSRT_None && LOGIC_StateRealTime != LSRT_WaitForConfig)
	{
		// Wait for direct current ready pulse from QPU
		if(LOGIC_StateRealTime == LSRT_DirectPulseStart && ZbGPIO_DCU1_Ready())
		{
			if(DataTable[REG_DIRECT_CURRENT] > DRCU_CURRENT_MAX)
			{
				if(ZbGPIO_DCU2_Ready())
				{
					if(DataTable[REG_DIRECT_CURRENT] > (DRCU_CURRENT_MAX + DRCU_CURRENT_MAX))
					{
						if(ZbGPIO_DCU3_Ready())
						{
							TimeReverseStart = LOGIC_RealTimeCounter + DC_CurrentPlateTicks;
							LOGIC_StateRealTime = LSRT_DirectPulseReady;
						}
					}
					else
					{
						TimeReverseStart = LOGIC_RealTimeCounter + DC_CurrentPlateTicks;
						LOGIC_StateRealTime = LSRT_DirectPulseReady;
					}
				}
			}
			else
			{
				TimeReverseStart = LOGIC_RealTimeCounter + DC_CurrentPlateTicks;
				LOGIC_StateRealTime = LSRT_DirectPulseReady;
			}
		}
		
		// Turn off gate driver
		if(LOGIC_RealTimeCounter >= LOGIC_DriverOffTicks)
		{
			ZbGPIO_DUT_Control(FALSE);
			ZbGPIO_DUT_Switch(FALSE);
		}
		
		// In case of direct current ready pulse timeout
		if(LOGIC_StateRealTime == LSRT_DirectPulseStart && LOGIC_RealTimeCounter > DC_READY_TIMEOUT_TICK)
		{
			ZbGPIO_DCU_Sync(FALSE);
			ZbGPIO_CSU_Sync(FALSE);
			LOGIC_StateRealTime = LSRT_None;
			
			if(++LOGIC_DCReadyRetries > DC_READY_RETRIES_NUM)
			{
				DataTable[REG_DC_READY_RETRIES] = LOGIC_DCReadyRetries;
				LOGIC_AbortMeasurement(WARNING_NO_DIRECT_CURRENT);
			}
		}
		
		// Start reverse current pulse and on-state voltage timer
		if(LOGIC_StateRealTime == LSRT_DirectPulseReady && LOGIC_RealTimeCounter > TimeReverseStart)
		{
			ZbGPIO_SCOPE_Sync(TRUE);
			ZbGPIO_RCU_Sync(TRUE);
			ZbGPIO_DCU_Sync(FALSE);
			ZbGPIO_DUT_Control(FALSE);
			ZbGPIO_DUT_Switch(FALSE);
			
			LOGIC_StateRealTime = LSRT_ReversePulseStart;
			TimeReverseStop = LOGIC_RealTimeCounter + OSV_ON_TIME_TICK;
			LOGIC_PreciseEventStart();
		}
		
		// Stop process
		if(LOGIC_StateRealTime == LSRT_ReversePulseStart && LOGIC_RealTimeCounter >= TimeReverseStop)
		{
			ZbGPIO_FCROVU_Sync(FALSE);
			ZbGPIO_SCOPE_Sync(FALSE);
			//
			ZbGPIO_RCU_Sync(FALSE);
			ZbGPIO_CSU_Sync(FALSE);
			
			TimeBeforeDataRead = LOGIC_RealTimeCounter + RT_DATA_READ_DELAY_TICK;
			LOGIC_StateRealTime = LSRT_ReadDataPause;
		}
		
		if(LOGIC_StateRealTime == LSRT_ReadDataPause && LOGIC_RealTimeCounter >= TimeBeforeDataRead)
		{
			Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC_SHORT;
			
			if(LOGIC_PulseNumRemain > 0)
				--LOGIC_PulseNumRemain;
			
			LOGIC_ReadDataPrepare();
			LOGIC_StateRealTime = LSRT_None;
			DCPulseFormed = TRUE;
		}
		
		++LOGIC_RealTimeCounter;
	}
}
// ----------------------------------------

Int16U LOGIC_GetPulsesRemain()
{
	return LOGIC_PulseNumRemain;
}
// ----------------------------------------

Int16U LOGIC_GetOpResult()
{
	return LOGIC_OperationResult;
}
// ----------------------------------------

Boolean LOGIC_DCPulseFormed()
{
	return DCPulseFormed;
}
// ----------------------------------------

Boolean LOGIC_DUTTriggered()
{
	return (ResultsCounter > 0) ? Results[ResultsCounter - 1].DeviceTriggered : TRUE;
}
// ----------------------------------------

void LOGIC_Halt()
{
	LOGIC_PulseNumRemain = 0;
	
	if(LOGIC_StateRealTime == LSRT_WaitForConfig)
		LOGIC_StateRealTime = LSRT_None;
	
	LOGIC_State = LS_None;
}
// ----------------------------------------

void LOGIC_TrimTrigTime(Boolean Increase)
{
	if(Increase)
		CROVU_TrigTime += CROVU_TrigTime_LastHalf;
	else
		CROVU_TrigTime -= CROVU_TrigTime_LastHalf;
	
	CROVU_TrigTime_LastHalf >>= 1;
}
// ----------------------------------------

void LOGIC_PreciseEventInit(Int16U usTime)
{
	ZwTimer_StopT1();
	(TqFastThyristor) ? ZwTimer_SetT1x10(usTime) : ZwTimer_SetT1(usTime);
}
// ----------------------------------------

void LOGIC_PreciseEventStart()
{
	// Avoid interrupts
	ZwTimer_StopT0();
	ZwTimer_StopT2();
	
	ZwTimer_StartT1();
}
// ----------------------------------------

void LOGIC_HandleCommunicationError()
{
	HLIError err = HLI_GetError();
	if(err.ErrorCode != ERR_NO_ERROR)
	{
		// Communiction error
		LOGIC_State = LS_Error;
		CONTROL_SwitchToFault(FAULT_PROTOCOL, 0);
	}
}
// ----------------------------------------

LogicState LOGIC_GetState()
{
	return LOGIC_State;
}
// ----------------------------------------

void LOGIC_ResetState()
{
	LOGIC_State = LS_None;
}
// ----------------------------------------

void LOGIC_CacheVariables()
{
	DCPulseFormed = FALSE;
	EmulateCROVU = DataTable[REG_EMULATE_CROVU];
	EmulateFCROVU = DataTable[REG_EMULATE_FCROVU];
	EmulateDCU1 = DataTable[REG_EMULATE_DCU1];
	EmulateDCU2 = DataTable[REG_EMULATE_DCU2];
	EmulateDCU3 = DataTable[REG_EMULATE_DCU3];
	EmulateRCU1 = DataTable[REG_EMULATE_RCU1];
	EmulateRCU2 = DataTable[REG_EMULATE_RCU2];
	EmulateRCU3 = DataTable[REG_EMULATE_RCU3];
	EmulateCSU = DataTable[REG_EMULATE_CSU];
	EmulateSCOPE = DataTable[REG_EMULATE_SCOPE];
	
	if(CacheUpdate)
	{
		TqFastThyristor = FALSE;
		DUTFinalIncrease = FALSE;
		LOGIC_OperationResult = OPRESULT_OK;
		
		LOGIC_DCReadyRetries = 0;
		ResultsCounter = 0;
		MeasurementMode = DataTable[REG_MODE];
		MuteCROVU = (MeasurementMode == MODE_QRR_ONLY) ? TRUE : FALSE;
		
		DC_Current = DataTable[REG_DIRECT_CURRENT];
		DC_CurrentRiseRate = DataTable[REG_DCU_RISE_RATE];
		DC_CurrentPlateTicks = DataTable[REG_DCU_PULSE_WIDTH] / TIMER2_PERIOD;
		DC_CurrentFallRate = DataTable[REG_DCU_FALL_RATE];
		DC_CurrentZeroPoint = DC_Current * 10 / DC_CurrentFallRate;
		DC_CurrentZeroPoint = (DC_CurrentZeroPoint > TQ_ZERO_OFFSET) ? (DC_CurrentZeroPoint - TQ_ZERO_OFFSET) : 0;
		
		RC_Current = DataTable[REG_DIRECT_CURRENT];
		RC_CurrentFallRate = DataTable[REG_DCU_FALL_RATE];
		
		CROVU_Voltage = DataTable[REG_OFF_STATE_VOLTAGE];
		CROVU_VoltageRate = DataTable[REG_OSV_RATE];

		FCROVU_IShortCircuit = DataTable[REG_FCROVU_I_SHORT];

		LOGIC_DriverOffTicks = (((DC_Current/DC_CurrentRiseRate / 2) > DC_DRIVER_OFF_DELAY_MIN) ?
									(DC_Current/DC_CurrentRiseRate / 2) : DC_DRIVER_OFF_DELAY_MIN) / TIMER2_PERIOD;

		if(MeasurementMode == MODE_QRR_ONLY)
		{
			LOGIC_PulseNumRemain = QRR_AVG_COUNTER;
			CROVU_TrigTime = DC_CurrentZeroPoint + TQ_MAX_TIME;
		}
		else if(CacheSinglePulse)
		{
			LOGIC_PulseNumRemain = 1;
			CROVU_TrigTime = DC_CurrentZeroPoint + DataTable[REG_TRIG_TIME];
		}
		else
		{
			LOGIC_PulseNumRemain = UNIT_TQ_MEASURE_PULSES;
			CROVU_TrigTime = DC_CurrentZeroPoint + TQ_FIRST_PROBE;
		}
		
		if(DataTable[REG_DCU1_ACTIVE])
			DC_CurrentMax = DRCU_CURRENT_MAX;
		if(DataTable[REG_DCU2_ACTIVE])
			DC_CurrentMax += DRCU_CURRENT_MAX;
		if(DataTable[REG_DCU3_ACTIVE])
			DC_CurrentMax += DRCU_CURRENT_MAX;
		
		if(DataTable[REG_RCU1_ACTIVE])
			RC_CurrentMax = DRCU_CURRENT_MAX;
		if(DataTable[REG_RCU2_ACTIVE])
			RC_CurrentMax += DRCU_CURRENT_MAX;
		if(DataTable[REG_RCU3_ACTIVE])
			RC_CurrentMax += DRCU_CURRENT_MAX;
		
		if(RC_Current > RC_CurrentMax)
		{
			RC_Current = RC_CurrentMax;
			DataTable[REG_WARNING] = WARNING_RC_SET_TO_HIGH;
		}
		
		if(DC_Current > DC_CurrentMax)
		{
			DC_Current = DC_CurrentMax;
			DataTable[REG_WARNING] = WARNING_DC_SET_TO_HIGH;
		}
		
		LOGIC_PreciseEventInit(CROVU_TrigTime);
		CacheUpdate = FALSE;
	}
}
// ----------------------------------------

void LOGIC_CacheUpdateSettings(Boolean UpdateMainSettings, Boolean SinglePulseMode)
{
	CacheUpdate = UpdateMainSettings;
	CacheSinglePulse = SinglePulseMode;
}
// ----------------------------------------

Boolean LOGIC_UpdateDeviceState()
{
	Int16U Register;
	
	if(!CMN_UpdateNodeState(EmulateCROVU, REG_CROVU_NODE_ID, &LOGIC_ExtDeviceState.DS_CROVU))
		return FALSE;

	if(!CMN_UpdateNodeState(EmulateFCROVU, REG_FCROVU_NODE_ID, &LOGIC_ExtDeviceState.DS_FCROVU))
		return FALSE;

	if(!CMN_UpdateNodeState(EmulateDCU1, REG_DCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU1))
		return FALSE;
	
	if(!CMN_UpdateNodeState(EmulateDCU2, REG_DCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU2))
		return FALSE;
	
	if(!CMN_UpdateNodeState(EmulateDCU3, REG_DCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU3))
		return FALSE;
	
	if(!CMN_UpdateNodeState(EmulateRCU1, REG_RCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU1))
		return FALSE;

	if(!CMN_UpdateNodeState(EmulateRCU2, REG_RCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU2))
		return FALSE;

	if(!CMN_UpdateNodeState(EmulateRCU3, REG_RCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU3))
		return FALSE;
	
	if(!EmulateSCOPE)
		if(HLI_RS232_Read16(REG_SCOPE_DEV_STATE, &Register))
			LOGIC_ExtDeviceState.DS_SCOPE = Register;
		else
			return FALSE;
	
	return TRUE;
}
// ----------------------------------------

void LOGIC_FaultResetPrepare()
{
	LOGIC_CacheVariables();
	LOGIC_State = LS_CLR_CROVU;
	//
	CONTROL_RequestDPC(&LOGIC_FaultResetSequence);
}
// ----------------------------------------

void LOGIC_FaultResetSequence()
{
	if(LOGIC_State == LS_CLR_CROVU || LOGIC_State == LS_CLR_FCROVU || LOGIC_State == LS_CLR_SCOPE
			|| LOGIC_State == LS_CLR_RCU1 || LOGIC_State == LS_CLR_RCU2 || LOGIC_State == LS_CLR_RCU3
			|| LOGIC_State == LS_CLR_DCU1 || LOGIC_State == LS_CLR_DCU2 || LOGIC_State == LS_CLR_DCU3)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch (LOGIC_State)
		{
			case LS_CLR_CROVU:
				CMN_ResetNodeFault(EmulateCROVU, REG_CROVU_NODE_ID, LOGIC_ExtDeviceState.DS_CROVU, &LOGIC_State,
						LS_CLR_FCROVU);
				break;
				
			case LS_CLR_FCROVU:
				CMN_ResetNodeFault(EmulateFCROVU, REG_FCROVU_NODE_ID, LOGIC_ExtDeviceState.DS_FCROVU, &LOGIC_State,
						LS_CLR_DCU1);
				break;

			case LS_CLR_DCU1:
				CMN_ResetNodeFault(EmulateDCU1, REG_DCU1_NODE_ID, LOGIC_ExtDeviceState.DS_DCU1, &LOGIC_State,
						LS_CLR_DCU2);
				break;

			case LS_CLR_DCU2:
				CMN_ResetNodeFault(EmulateDCU2, REG_DCU2_NODE_ID, LOGIC_ExtDeviceState.DS_DCU2, &LOGIC_State,
						LS_CLR_DCU3);
				break;

			case LS_CLR_DCU3:
				CMN_ResetNodeFault(EmulateDCU3, REG_DCU3_NODE_ID, LOGIC_ExtDeviceState.DS_DCU3, &LOGIC_State,
						LS_CLR_RCU1);
				break;

			case LS_CLR_RCU1:
				CMN_ResetNodeFault(EmulateRCU1, REG_RCU1_NODE_ID, LOGIC_ExtDeviceState.DS_RCU1, &LOGIC_State,
						LS_CLR_RCU2);
				break;
				
			case LS_CLR_RCU2:
				CMN_ResetNodeFault(EmulateRCU2, REG_RCU2_NODE_ID, LOGIC_ExtDeviceState.DS_RCU2, &LOGIC_State,
						LS_CLR_RCU3);
				break;
				
			case LS_CLR_RCU3:
				CMN_ResetNodeFault(EmulateRCU3, REG_RCU3_NODE_ID, LOGIC_ExtDeviceState.DS_RCU3, &LOGIC_State,
						LS_CLR_SCOPE);
				break;
				
			case LS_CLR_SCOPE:
				if(!EmulateSCOPE && (LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_FAULT))
				{
					if(HLI_RS232_CallAction(ACT_SCOPE_CLR_FAULT))
						LOGIC_State = LS_None;
				}
				else
					LOGIC_State = LS_None;
				break;
		}
		
		LOGIC_HandleCommunicationError();
	}
	else
		CONTROL_RequestDPC(NULL);
}
// ----------------------------------------

void LOGIC_PowerOnPrepare()
{
	LOGIC_CacheVariables();
	LOGIC_State = LS_PON_CROVU;
	//
	CONTROL_RequestDPC(&LOGIC_PowerOnSequence);
}
// ----------------------------------------

void LOGIC_PowerOnSequence()
{
	if(LOGIC_State == LS_PON_CROVU || LOGIC_State == LS_PON_FCROVU || LOGIC_State == LS_PON_DCU
			|| LOGIC_State == LS_PON_RCU || LOGIC_State == LS_PON_CSU || LOGIC_State == LS_PON_SCOPE
			|| LOGIC_State == LS_PON_WaitStates)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		if(LOGIC_State != LS_PON_WaitStates)
		{
			if(CONTROL_TimeCounter <= (UnitsStartTimeOut + TIMEOUT_UNITS_STARTUP))
				return;
			
			UnitsStartTimeOut = CONTROL_TimeCounter;
		}
		
		switch (LOGIC_State)
		{
			case LS_PON_CROVU:
				{
					// Handle CROVU node
					if(!EmulateCROVU)
					{
						switch (LOGIC_ExtDeviceState.DS_CROVU)
						{
							case DS_CROVU_NONE:
								if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_ENABLE_POWER))
									LOGIC_State = LS_PON_FCROVU;
								break;
							case DS_CROVU_READY:
								if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_DISABLE_POWER))
									if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_ENABLE_POWER))
										LOGIC_State = LS_PON_FCROVU;
								break;
							case DS_CROVU_FAULT:
								if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_CLR_FAULT))
									if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_ENABLE_POWER))
										LOGIC_State = LS_PON_FCROVU;
								break;
							case DS_CROVU_DISABLED:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_PON_WRONG_STATE);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_CROVU = DS_CROVU_READY;
						LOGIC_State = LS_PON_FCROVU;
					}
				}
				break;
				
			case LS_PON_FCROVU:
				{
					// Handle FCROVU node
					if(!EmulateFCROVU)
					{
						switch (LOGIC_ExtDeviceState.DS_FCROVU)
						{
							case DS_FCROVU_NONE:
								if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_ENABLE_POWER))
									LOGIC_State = LS_PON_DCU;
								break;
							case DS_FCROVU_READY:
								if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_DISABLE_POWER))
									if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_ENABLE_POWER))
										LOGIC_State = LS_PON_DCU;
								break;
							case DS_FCROVU_FAULT:
								if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_FAULT_CLEAR))
									if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_ENABLE_POWER))
										LOGIC_State = LS_PON_DCU;
								break;
							case DS_FCROVU_DISABLED:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_PON_WRONG_STATE);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_FCROVU = DS_FCROVU_READY;
						LOGIC_State = LS_PON_DCU;
					}
				}
				break;
				
			case LS_PON_DCU:
				{
					if(!EmulateDCU1)
					{
						if(DataTable[REG_DCU1_ACTIVE])
						{
							switch (LOGIC_ExtDeviceState.DS_DCU1)
							{
								case DS_DCU_NONE:
									if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_ENABLE_POWER))
										return;
								case DS_DCU_FAULT:
									if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_FAULT_CLEAR))
										if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_ENABLE_POWER))
											return;
								case DS_DCU_DISABLED:
									LOGIC_State = LS_Error;
									CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, FAULTEX_PON_WRONG_STATE);
									break;
							}
						}
						
						if(DataTable[REG_DCU2_ACTIVE])
						{
							switch (LOGIC_ExtDeviceState.DS_DCU2)
							{
								case DS_DCU_NONE:
									if(HLI_CAN_CallAction(DataTable[REG_DCU2_NODE_ID], ACT_DCU_ENABLE_POWER))
										return;
								case DS_DCU_FAULT:
									if(HLI_CAN_CallAction(DataTable[REG_DCU2_NODE_ID], ACT_DCU_FAULT_CLEAR))
										if(HLI_CAN_CallAction(DataTable[REG_DCU2_NODE_ID], ACT_DCU_ENABLE_POWER))
											return;
								case DS_DCU_DISABLED:
									LOGIC_State = LS_Error;
									CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, FAULTEX_PON_WRONG_STATE);
									break;
							}
						}
						
						if(DataTable[REG_DCU3_ACTIVE])
						{
							switch (LOGIC_ExtDeviceState.DS_DCU3)
							{
								case DS_DCU_NONE:
									if(HLI_CAN_CallAction(DataTable[REG_DCU3_NODE_ID], ACT_DCU_ENABLE_POWER))
										return;
								case DS_DCU_FAULT:
									if(HLI_CAN_CallAction(DataTable[REG_DCU3_NODE_ID], ACT_DCU_FAULT_CLEAR))
										if(HLI_CAN_CallAction(DataTable[REG_DCU3_NODE_ID], ACT_DCU_ENABLE_POWER))
											return;
								case DS_DCU_DISABLED:
									LOGIC_State = LS_Error;
									CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, FAULTEX_PON_WRONG_STATE);
									break;
							}
						}
						
						LOGIC_State = LS_PON_RCU;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_DCU1 = DS_DCU_READY;
						LOGIC_ExtDeviceState.DS_DCU2 = DS_DCU_READY;
						LOGIC_ExtDeviceState.DS_DCU3 = DS_DCU_READY;
						LOGIC_State = LS_PON_RCU;
					}
				}
				break;
				
			case LS_PON_RCU:
				{
					if(!EmulateRCU1)
					{
						if(DataTable[REG_RCU1_ACTIVE])
						{
							switch (LOGIC_ExtDeviceState.DS_RCU1)
							{
								case DS_RCU_NONE:
									if(HLI_CAN_CallAction(DataTable[REG_RCU1_NODE_ID], ACT_RCU_ENABLE_POWER))
										return;
								case DS_RCU_FAULT:
									if(HLI_CAN_CallAction(DataTable[REG_RCU1_NODE_ID], ACT_RCU_FAULT_CLEAR))
										if(HLI_CAN_CallAction(DataTable[REG_RCU1_NODE_ID], ACT_RCU_ENABLE_POWER))
											return;
								case DS_RCU_DISABLED:
									LOGIC_State = LS_Error;
									CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, FAULTEX_PON_WRONG_STATE);
									break;
							}
						}
						
						if(DataTable[REG_RCU2_ACTIVE])
						{
							switch (LOGIC_ExtDeviceState.DS_RCU2)
							{
								case DS_RCU_NONE:
									if(HLI_CAN_CallAction(DataTable[REG_RCU2_NODE_ID], ACT_RCU_ENABLE_POWER))
										return;
								case DS_RCU_FAULT:
									if(HLI_CAN_CallAction(DataTable[REG_RCU2_NODE_ID], ACT_RCU_FAULT_CLEAR))
										if(HLI_CAN_CallAction(DataTable[REG_RCU2_NODE_ID], ACT_RCU_ENABLE_POWER))
											return;
								case DS_RCU_DISABLED:
									LOGIC_State = LS_Error;
									CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, FAULTEX_PON_WRONG_STATE);
									break;
							}
						}
						
						if(DataTable[REG_RCU3_ACTIVE])
						{
							switch (LOGIC_ExtDeviceState.DS_RCU3)
							{
								case DS_RCU_NONE:
									if(HLI_CAN_CallAction(DataTable[REG_RCU3_NODE_ID], ACT_RCU_ENABLE_POWER))
										return;
								case DS_RCU_FAULT:
									if(HLI_CAN_CallAction(DataTable[REG_RCU3_NODE_ID], ACT_RCU_FAULT_CLEAR))
										if(HLI_CAN_CallAction(DataTable[REG_RCU3_NODE_ID], ACT_RCU_ENABLE_POWER))
											return;
								case DS_RCU_DISABLED:
									LOGIC_State = LS_Error;
									CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, FAULTEX_PON_WRONG_STATE);
									break;
							}
						}
						
						LOGIC_State = LS_PON_CSU;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_RCU1 = DS_RCU_READY;
						LOGIC_ExtDeviceState.DS_RCU2 = DS_RCU_READY;
						LOGIC_ExtDeviceState.DS_RCU3 = DS_RCU_READY;
						LOGIC_State = LS_PON_CSU;
					}
				}
				break;
				
			case LS_PON_CSU:
				{
					if(!EmulateCSU)
					{
						ZbGPIO_CSU_PWRCtrl(TRUE);
					}
					else
					{
						LOGIC_ExtDeviceState.DS_CSU = DS_CSU_READY;
						LOGIC_State = LS_PON_SCOPE;
					}
				}
				break;
				
			case LS_PON_SCOPE:
				{
					// Handle SCOPE node
					if(!EmulateSCOPE)
					{
						switch (LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case DS_SCOPE_FAULT:
								if(HLI_RS232_CallAction(ACT_SCOPE_CLR_FAULT))
									LOGIC_State = LS_PON_WaitStates;
								break;
							case DS_SCOPE_DISABLED:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_PON_WRONG_STATE);
								break;
							case DS_SCOPE_IN_PROCESS:
								if(HLI_RS232_CallAction(ACT_SCOPE_STOP_TEST))
									LOGIC_State = LS_PON_WaitStates;
								break;
							default:
								LOGIC_State = LS_PON_WaitStates;
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_SCOPE = DS_SCOPE_NONE;
						LOGIC_State = LS_PON_WaitStates;
					}
					
					if(LOGIC_State != LS_PON_SCOPE)
						Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC;
				}
				break;
				
			case LS_PON_WaitStates:
				{
					if(Timeout > CONTROL_TimeCounter)
					{
						if(LOGIC_ExtDeviceState.DS_CROVU == DS_CROVU_READY
								&& LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_READY
								&& LOGIC_ExtDeviceState.DS_DCU1 == DS_DCU_READY
								&& LOGIC_ExtDeviceState.DS_DCU2 == DS_DCU_READY
								&& LOGIC_ExtDeviceState.DS_DCU3 == DS_DCU_READY
								&& LOGIC_ExtDeviceState.DS_RCU1 == DS_RCU_READY
								&& LOGIC_ExtDeviceState.DS_RCU2 == DS_RCU_READY
								&& LOGIC_ExtDeviceState.DS_RCU3 == DS_RCU_READY
								&& LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_NONE)
						{
							LOGIC_State = LS_None;
						}
					}
					else
					{
						if(LOGIC_ExtDeviceState.DS_CROVU != DS_CROVU_READY)
						{
							// CROVU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_FCROVU != DS_FCROVU_READY)
						{
							// FCROVU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_DCU1 != DS_DCU_READY)
						{
							// DCU1 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_DCU2 != DS_DCU_READY)
						{
							// DCU2 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_DCU3 != DS_DCU_READY)
						{
							// DCU3 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_RCU1 != DS_RCU_READY)
						{
							// RCU1 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_RCU2 != DS_RCU_READY)
						{
							// RCU2 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_RCU3 != DS_RCU_READY)
						{
							// RCU3 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_SCOPE != DS_SCOPE_NONE)
						{
							// SCOPE not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_PON_TIMEOUT);
						}
						else
							CONTROL_SwitchToFault(FAULT_LOGIC_GENERAL, FAULTEX_PON_TIMEOUT);
						
						LOGIC_State = LS_Error;
					}
				}
				break;
		}
		
		LOGIC_HandleCommunicationError();
	}
	else
		CONTROL_RequestDPC(NULL);
}
// ----------------------------------------

void LOGIC_ConfigurePrepare()
{
	LOGIC_CacheVariables();
	LOGIC_State = LS_CFG_CROVU;
	//
	CONTROL_RequestDPC(&LOGIC_ConfigureSequence);
}
// ----------------------------------------

void LOGIC_ConfigureSequence()
{
	if(LOGIC_State == LS_CFG_CROVU || LOGIC_State == LS_CFG_FCROVU || LOGIC_State == LS_CFG_DCU
			|| LOGIC_State == LS_CFG_RCU || LOGIC_State == LS_CFG_SCOPE || LOGIC_State == LS_CFG_WaitStates)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch (LOGIC_State)
		{
			case LS_CFG_CROVU:
				{
					// Handle CROVU node
					if(!EmulateCROVU && !MuteCROVU)
					{
						if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_ENABLE_EXT_SYNC))
							if(HLI_CAN_Write16(DataTable[REG_CROVU_NODE_ID], REG_CROVU_DESIRED_VOLTAGE, CROVU_Voltage))
								if(HLI_CAN_Write16(DataTable[REG_CROVU_NODE_ID], REG_CROVU_VOLTAGE_RATE, CROVU_VoltageRate))
									if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_APPLY_SETTINGS))
										LOGIC_State = LS_CFG_FCROVU;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_CROVU = DS_CROVU_READY;
						LOGIC_State = LS_CFG_FCROVU;
					}
				}
				break;
				
			case LS_CFG_FCROVU:
				{
					// Handle FCROVU node
					if(!EmulateFCROVU && !MuteCROVU)
					{
						if(HLI_CAN_Write16(DataTable[REG_FCROVU_NODE_ID], REG_FCROVU_V_RATE_VALUE, CROVU_VoltageRate))
							if(HLI_CAN_Write16(DataTable[REG_FCROVU_NODE_ID], REG_FCROVU_I_SHORT_CIRCUIT, FCROVU_IShortCircuit))
								if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_CONFIG))
									LOGIC_State = LS_CFG_DCU;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_FCROVU = DS_FCROVU_READY;
						LOGIC_State = LS_CFG_DCU;
					}
				}
				break;
				
			case LS_CFG_DCU:
				{
					// Handle DCU node
					if(!EmulateDCU1)
					{
						if((LOGIC_ExtDeviceState.DS_DCU3 == DS_DCU_READY && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if(DC_Current >= DRCU_CURRENT_MAX)
							{
								if(HLI_CAN_Write16(DataTable[REG_DCU1_NODE_ID], REG_DCU_I_MAX_VALUE, DRCU_CURRENT_MAX))
									if(HLI_CAN_Write16(DataTable[REG_DCU1_NODE_ID], REG_DCU_I_RATE_RISE, DC_CurrentRiseRate))
										if(HLI_CAN_Write16(DataTable[REG_DCU1_NODE_ID], REG_DCU_I_RATE_FALL, DC_CurrentFallRate))
											if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_CONFIG))
												DC_Current = DC_Current - DRCU_CURRENT_MAX;
							}
						}
						
						if((LOGIC_ExtDeviceState.DS_DCU2 == DS_DCU_READY && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if(DC_Current >= DRCU_CURRENT_MAX)
							{
								if(HLI_CAN_Write16(DataTable[REG_DCU2_NODE_ID], REG_DCU_I_MAX_VALUE, DRCU_CURRENT_MAX))
									if(HLI_CAN_Write16(DataTable[REG_DCU2_NODE_ID], REG_DCU_I_RATE_RISE, DC_CurrentRiseRate))
										if(HLI_CAN_Write16(DataTable[REG_DCU2_NODE_ID], REG_DCU_I_RATE_FALL, DC_CurrentFallRate))
											if(HLI_CAN_CallAction(DataTable[REG_DCU2_NODE_ID], ACT_DCU_CONFIG))
												DC_Current = DC_Current - DRCU_CURRENT_MAX;
							}
						}
						
						if((LOGIC_ExtDeviceState.DS_DCU1 == DS_DCU_READY && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if(HLI_CAN_Write16(DataTable[REG_DCU1_NODE_ID], REG_DCU_I_MAX_VALUE, DC_CurrentMax))
								if(HLI_CAN_Write16(DataTable[REG_DCU1_NODE_ID], REG_DCU_I_RATE_RISE, DC_CurrentRiseRate))
									if(HLI_CAN_Write16(DataTable[REG_DCU1_NODE_ID], REG_DCU_I_RATE_FALL, DC_CurrentFallRate))
										if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_CONFIG))
										{
											LOGIC_State = LS_PON_RCU;
											DC_Current = 0;
										}
						}
						
					}
					else
					{
						LOGIC_ExtDeviceState.DS_DCU1 = DS_DCU_CONFIG_READY;
						LOGIC_ExtDeviceState.DS_DCU2 = DS_DCU_CONFIG_READY;
						LOGIC_ExtDeviceState.DS_DCU3 = DS_DCU_CONFIG_READY;
						LOGIC_State = LS_CFG_RCU;
					}
				}
				break;
				
			case LS_CFG_RCU:
				{
					// Handle RCU node
					if(!EmulateRCU1)
					{
						if((LOGIC_ExtDeviceState.DS_RCU3 == DS_RCU_READY && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if(RC_Current >= DRCU_CURRENT_MAX)
							{
								if(HLI_CAN_Write16(DataTable[REG_RCU1_NODE_ID], REG_DCU_I_MAX_VALUE, DRCU_CURRENT_MAX))
									if(HLI_CAN_Write16(DataTable[REG_RCU1_NODE_ID], REG_RCU_I_RATE_FALL, RC_CurrentFallRate))
										if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_CONFIG))
											RC_Current = RC_Current - DRCU_CURRENT_MAX;
							}
						}
						
						if((LOGIC_ExtDeviceState.DS_RCU2 == DS_RCU_READY && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if(RC_Current >= DRCU_CURRENT_MAX)
							{
								if(HLI_CAN_Write16(DataTable[REG_RCU2_NODE_ID], REG_DCU_I_MAX_VALUE, DRCU_CURRENT_MAX))
									if(HLI_CAN_Write16(DataTable[REG_RCU2_NODE_ID], REG_RCU_I_RATE_FALL, RC_CurrentFallRate))
										if(HLI_CAN_CallAction(DataTable[REG_DCU2_NODE_ID], ACT_DCU_CONFIG))
											RC_Current = RC_Current - DRCU_CURRENT_MAX;
							}
						}
						
						if((LOGIC_ExtDeviceState.DS_RCU1 == DS_RCU_READY && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if(HLI_CAN_Write16(DataTable[REG_RCU1_NODE_ID], REG_DCU_I_MAX_VALUE, DRCU_CURRENT_MAX))
								if(HLI_CAN_Write16(DataTable[REG_RCU1_NODE_ID], REG_RCU_I_RATE_FALL, RC_CurrentFallRate))
									if(HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_CONFIG))
									{
										LOGIC_State = LS_CFG_SCOPE;
										RC_Current = 0;
									}
						}
						
					}
					else
					{
						LOGIC_ExtDeviceState.DS_RCU1 = DS_DCU_CONFIG_READY;
						LOGIC_ExtDeviceState.DS_RCU2 = DS_DCU_CONFIG_READY;
						LOGIC_ExtDeviceState.DS_RCU3 = DS_DCU_CONFIG_READY;
						LOGIC_State = LS_CFG_SCOPE;
					}
				}
				break;
				
			case LS_CFG_SCOPE:
				{
					// Handle SCOPE node
					if(!EmulateSCOPE)
					{
						switch (LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case DS_SCOPE_NONE:
								{
									if(HLI_RS232_Write16(REG_SCOPE_CURRENT_AMPL, DataTable[REG_DIRECT_CURRENT]))
										if(HLI_RS232_Write16(REG_SCOPE_MEASURE_MODE, MeasurementMode))
											if(HLI_RS232_Write16(REG_SCOPE_TR_050_METHOD, DataTable[REG_TRR_DETECTION_MODE]))
												if(HLI_RS232_CallAction(ACT_SCOPE_START_TEST))
													LOGIC_State = LS_CFG_WaitStates;
								}
								break;
							case DS_SCOPE_IN_PROCESS:
								HLI_RS232_CallAction(ACT_SCOPE_STOP_TEST);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_SCOPE = DS_SCOPE_IN_PROCESS;
						LOGIC_State = LS_CFG_WaitStates;
					}
					
					if(LOGIC_State != LS_CFG_SCOPE)
						Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC;
				}
				break;
				
			case LS_CFG_WaitStates:
				{
					if(Timeout > CONTROL_TimeCounter)
					{
						if(LOGIC_ExtDeviceState.DS_CROVU == DS_CROVU_READY
								&& LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_READY
								&& LOGIC_ExtDeviceState.DS_DCU1 == DS_DCU_CONFIG_READY
								&& LOGIC_ExtDeviceState.DS_DCU2 == DS_DCU_CONFIG_READY
								&& LOGIC_ExtDeviceState.DS_DCU3 == DS_DCU_CONFIG_READY
								&& LOGIC_ExtDeviceState.DS_RCU1 == DS_DCU_CONFIG_READY
								&& LOGIC_ExtDeviceState.DS_RCU2 == DS_DCU_CONFIG_READY
								&& LOGIC_ExtDeviceState.DS_RCU3 == DS_DCU_CONFIG_READY
								&& LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_IN_PROCESS)
						{
							LOGIC_State = LS_None;
						}
					}
					else
					{
						if(LOGIC_ExtDeviceState.DS_CROVU != DS_CROVU_READY)
						{
							// CROVU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_FCROVU != DS_FCROVU_READY)
						{
							// FCROVU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_DCU1 != DS_DCU_CONFIG_READY)
						{
							// DCU1 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_DCU2 != DS_DCU_CONFIG_READY)
						{
							// DCU2 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_DCU3 != DS_DCU_CONFIG_READY)
						{
							// DCU3 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_RCU1 != DS_RCU_CONFIG_READY)
						{
							// RCU1 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_RCU2 != DS_RCU_CONFIG_READY)
						{
							// RCU2 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_RCU3 != DS_RCU_CONFIG_READY)
						{
							// RCU3 not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, FAULTEX_PON_TIMEOUT);
						}
						else if(LOGIC_ExtDeviceState.DS_SCOPE != DS_SCOPE_NONE)
						{
							// SCOPE not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_PON_TIMEOUT);
						}
						else
							CONTROL_SwitchToFault(FAULT_LOGIC_GENERAL, FAULTEX_PON_TIMEOUT);
						
						LOGIC_State = LS_Error;
					}
				}
				break;
		}
		
		LOGIC_HandleCommunicationError();
	}
	else
		CONTROL_RequestDPC(NULL);
}
// ----------------------------------------

void LOGIC_PowerOffPrepare()
{
	LOGIC_CacheVariables();
	LOGIC_State = LS_POFF_CROVU;
	//
	CONTROL_RequestDPC(&LOGIC_PowerOffSequence);
}
// ----------------------------------------

void LOGIC_PowerOffSequence()
{
	if(LOGIC_State == LS_POFF_CROVU || LOGIC_State == LS_POFF_FCROVU || LOGIC_State == LS_POFF_DCU
			|| LOGIC_State == LS_POFF_RCU || LOGIC_State == LS_POFF_CSU || LOGIC_State == LS_POFF_SCOPE)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch (LOGIC_State)
		{
			case LS_POFF_CROVU:
				{
					// Handle CROVU node
					if(!EmulateCROVU)
					{
						switch (LOGIC_ExtDeviceState.DS_CROVU)
						{
							case DS_CROVU_NONE:
							case DS_CROVU_READY:
								if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_DISABLE_POWER))
									LOGIC_State = LS_POFF_FCROVU;
								break;
							default:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, FAULTEX_POFF_WRONG_STATE);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_CROVU = DS_CROVU_NONE;
						LOGIC_State = LS_POFF_FCROVU;
					}
				}
				break;
				
			case LS_POFF_FCROVU:
				{
					// Handle FCROVU node
					if(!EmulateFCROVU)
					{
						switch (LOGIC_ExtDeviceState.DS_FCROVU)
						{
							case DS_FCROVU_NONE:
							case DS_FCROVU_READY:
								if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_DISABLE_POWER))
									LOGIC_State = LS_POFF_DCU;
								break;
							default:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_POFF_WRONG_STATE);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_FCROVU = DS_FCROVU_NONE;
						LOGIC_State = LS_POFF_DCU;
					}
				}
				break;
				
			case LS_POFF_DCU:
				{
					if(!EmulateDCU1)
					{
						switch (LOGIC_ExtDeviceState.DS_DCU1)
						{
							case DS_DCU_FAULT:
							case DS_DCU_DISABLED:
							case DS_DCU_IN_PROCESS:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, FAULTEX_POFF_WRONG_STATE);
								break;
							default:
								HLI_CAN_CallAction(DataTable[REG_DCU1_NODE_ID], ACT_DCU_DISABLE_POWER);
								break;
						}
						
						switch (LOGIC_ExtDeviceState.DS_DCU2)
						{
							case DS_DCU_FAULT:
							case DS_DCU_DISABLED:
							case DS_DCU_IN_PROCESS:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, FAULTEX_POFF_WRONG_STATE);
								break;
							default:
								HLI_CAN_CallAction(DataTable[REG_DCU2_NODE_ID], ACT_DCU_DISABLE_POWER);
								break;
						}
						
						switch (LOGIC_ExtDeviceState.DS_DCU3)
						{
							case DS_DCU_FAULT:
							case DS_DCU_DISABLED:
							case DS_DCU_IN_PROCESS:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, FAULTEX_POFF_WRONG_STATE);
								break;
							default:
								HLI_CAN_CallAction(DataTable[REG_DCU3_NODE_ID], ACT_DCU_DISABLE_POWER);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_DCU1 = DS_DCU_NONE;
						LOGIC_ExtDeviceState.DS_DCU2 = DS_DCU_NONE;
						LOGIC_ExtDeviceState.DS_DCU3 = DS_DCU_NONE;
						LOGIC_State = LS_POFF_RCU;
					}
				}
				break;
				
			case LS_POFF_RCU:
				{
					if(!EmulateRCU1)
					{
						switch (LOGIC_ExtDeviceState.DS_RCU1)
						{
							case DS_RCU_FAULT:
							case DS_RCU_DISABLED:
							case DS_RCU_IN_PROCESS:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, FAULTEX_POFF_WRONG_STATE);
								break;
							default:
								HLI_CAN_CallAction(DataTable[REG_RCU1_NODE_ID], ACT_RCU_DISABLE_POWER);
								break;
						}
						
						switch (LOGIC_ExtDeviceState.DS_RCU2)
						{
							case DS_RCU_FAULT:
							case DS_RCU_DISABLED:
							case DS_RCU_IN_PROCESS:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, FAULTEX_POFF_WRONG_STATE);
								break;
							default:
								HLI_CAN_CallAction(DataTable[REG_RCU2_NODE_ID], ACT_RCU_DISABLE_POWER);
								break;
						}
						
						switch (LOGIC_ExtDeviceState.DS_RCU3)
						{
							case DS_RCU_FAULT:
							case DS_RCU_DISABLED:
							case DS_RCU_IN_PROCESS:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, FAULTEX_POFF_WRONG_STATE);
								break;
							default:
								HLI_CAN_CallAction(DataTable[REG_RCU3_NODE_ID], ACT_RCU_DISABLE_POWER);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_RCU1 = DS_RCU_NONE;
						LOGIC_ExtDeviceState.DS_RCU2 = DS_RCU_NONE;
						LOGIC_ExtDeviceState.DS_RCU3 = DS_RCU_NONE;
						LOGIC_State = LS_POFF_SCOPE;
					}
				}
				break;
				
			case LS_POFF_SCOPE:
				{
					// Handle SCOPE node
					if(!EmulateSCOPE)
					{
						switch (LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case DS_SCOPE_IN_PROCESS:
								if(HLI_RS232_CallAction(ACT_SCOPE_STOP_TEST))
									LOGIC_State = LS_None;
								break;
							default:
								LOGIC_State = LS_None;
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_SCOPE = DS_SCOPE_NONE;
						LOGIC_State = LS_None;
					}
				}
				break;
		}
		
		LOGIC_HandleCommunicationError();
	}
	else
		CONTROL_RequestDPC(NULL);
	
}
// ----------------------------------------

void LOGIC_ReadDataPrepare()
{
	LOGIC_State = LS_READ_CROVU;
	CONTROL_RequestDPC(&LOGIC_ReadDataSequence);
}
// ----------------------------------------

void LOGIC_ReadDataSequence()
{
	Int16U Register;
	
	if(LOGIC_State == LS_READ_CROVU || LOGIC_State == LS_READ_DCU || LOGIC_State == LS_READ_RCU
			|| LOGIC_State == LS_READ_SCOPE)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch (LOGIC_State)
		{
			case LS_READ_CROVU:
				{
					if(!EmulateCROVU)
					{
						if(LOGIC_ExtDeviceState.DS_CROVU == DS_CROVU_READY)
							if(HLI_CAN_Read16(DataTable[REG_CROVU_NODE_ID], REG_CROVU_TEST_RESULT, &Register))
							{
								if(MeasurementMode == MODE_QRR_TQ)
								{
									if(Register == OPRESULT_NONE)
									{
										LOGIC_State = LS_Error;
										CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, FAULTEX_READ_WRONG_STATE);
									}
									else
									{
										Results[ResultsCounter].DeviceTriggered =
												(Register == OPRESULT_OK) ? FALSE : TRUE;
										LOGIC_State = LS_READ_FCROVU;
									}
								}
								else
									LOGIC_State = LS_READ_FCROVU;
							}
					}
					else
						LOGIC_State = LS_READ_FCROVU;
				}
				break;
				
			case LS_READ_FCROVU:
				{
					if(!EmulateFCROVU)
					{
						if(LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_READY)
							if(HLI_CAN_Read16(DataTable[REG_FCROVU_NODE_ID], REG_FCROVU_TEST_FINISHED, &Register))
							{
								if(MeasurementMode == MODE_QRR_TQ)
								{
									if(Register == OPRESULT_NONE)
									{
										LOGIC_State = LS_Error;
										CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_READ_WRONG_STATE);
									}
									else
									{
										Results[ResultsCounter].DeviceTriggered =
												(Register == OPRESULT_OK) ? FALSE : TRUE;
										LOGIC_State = LS_READ_DCU;
									}
								}
								else
									LOGIC_State = LS_READ_DCU;
							}
					}
					else
						LOGIC_State = LS_READ_DCU;
				}
				break;
				
			case LS_READ_DCU:
				{
					if(!EmulateDCU1)
					{
						Int16U Idc1Temp = 0, Idc2Temp = 0, Idc3Temp = 0;
						
						LOGIC_State = LS_READ_SCOPE;
						
						if(DataTable[REG_DCU1_ACTIVE])
							if(LOGIC_ExtDeviceState.DS_DCU1 == DS_DCU_CHARGING)
								if(!HLI_CAN_Read16(DataTable[REG_DCU1_NODE_ID], REG_DCU_DBG_I_MAX_DUT_VALUE, &Idc1Temp))
									LOGIC_State = LS_READ_DCU;
						
						if(DataTable[REG_DCU2_ACTIVE])
							if(LOGIC_ExtDeviceState.DS_DCU2 == DS_DCU_CHARGING)
								if(!HLI_CAN_Read16(DataTable[REG_DCU2_NODE_ID], REG_DCU_DBG_I_MAX_DUT_VALUE, &Idc2Temp))
									LOGIC_State = LS_READ_DCU;
						
						if(DataTable[REG_DCU3_ACTIVE])
							if(LOGIC_ExtDeviceState.DS_DCU3 == DS_DCU_CHARGING)
								if(!HLI_CAN_Read16(DataTable[REG_DCU3_NODE_ID], REG_DCU_DBG_I_MAX_DUT_VALUE, &Idc3Temp))
									LOGIC_State = LS_READ_DCU;
						
						Results[ResultsCounter].Idc = Idc1Temp + Idc2Temp + Idc3Temp;
					}
					else
						LOGIC_State = LS_READ_SCOPE;
				}
				break;
				
			case LS_READ_SCOPE:
				{
					if(!EmulateSCOPE)
					{
						Int16U Problem;
						
						if(LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_NONE)
							if(HLI_RS232_Read16(REG_SCOPE_FINISHED, &Register))
								if(HLI_RS232_Read16(REG_SCOPE_PROBLEM, &Problem))
									if(HLI_RS232_Read16(REG_SCOPE_RESULT_IRR, &Results[ResultsCounter].Irr))
										if(HLI_RS232_Read16(REG_SCOPE_RESULT_TRR, &Results[ResultsCounter].Trr))
											if(HLI_RS232_Read16(REG_SCOPE_RESULT_QRR, &Results[ResultsCounter].Qrr))
												if(HLI_RS232_Read16(REG_SCOPE_RESULT_ZERO, &Results[ResultsCounter].ZeroI))
													if(HLI_RS232_Read16(REG_SCOPE_RESULT_ZERO_V, &Results[ResultsCounter].ZeroV))
														if(HLI_RS232_Read16(REG_SCOPE_RESULT_DIDT, &Results[ResultsCounter].dIdt))
														{
															if(Register == OPRESULT_NONE)
															{
																LOGIC_State = LS_Error;
																CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE,
																		FAULTEX_READ_WRONG_STATE);
															}
															else if((Register == OPRESULT_FAIL
																	&& Problem != PROBLEM_SCOPE_CALC_VZ)
																	|| (Register == OPRESULT_FAIL
																			&& Problem == PROBLEM_SCOPE_CALC_VZ
																			&& !Results[ResultsCounter].DeviceTriggered))
															{
																LOGIC_AbortMeasurement(WARNING_SCOPE_CALC_FAILED);
															}
															else if(Results[ResultsCounter].Irr > DC_Current)
															{
																LOGIC_AbortMeasurement(WARNING_IRR_TO_HIGH);
															}
															else
															{
																// Save results
																Results[ResultsCounter].OSVApplyTime = CROVU_TrigTime;
																LOGIC_LogData(Results[ResultsCounter]);
																
																// Apply extended Tq logic
																if(MeasurementMode == MODE_QRR_TQ && !CacheSinglePulse)
																	LOGIC_TqExtraLogic(
																			Results[ResultsCounter].DeviceTriggered);
																
																LOGIC_State = LS_None;
																
																DataTable[REG_PULSES_COUNTER] = ++ResultsCounter;
															}
														}
					}
					else
					{
						++ResultsCounter;
						LOGIC_State = LS_None;
					}
				}
				break;
		}
		
		if(CONTROL_TimeCounter > Timeout && LOGIC_State != LS_None)
		{
			if(LOGIC_ExtDeviceState.DS_CROVU != DS_CROVU_READY)
			{
				// CROVU not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_CROVU, FAULTEX_READ_TIMEOUT);
			}
			if(LOGIC_ExtDeviceState.DS_FCROVU != DS_FCROVU_READY)
			{
				// FCROVU not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_DCU1 != DS_DCU_READY)
			{
				// DCU1 not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU1, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_DCU2 != DS_DCU_READY)
			{
				// DCU2 not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU2, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_DCU3 != DS_DCU_READY)
			{
				// DCU3 not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_DCU3, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_RCU1 != DS_DCU_READY)
			{
				// RCU1 not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU1, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_RCU2 != DS_DCU_READY)
			{
				// RCU2 not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU2, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_RCU3 != DS_DCU_READY)
			{
				// RCU3 not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_RCU3, FAULTEX_READ_TIMEOUT);
			}
			else if(LOGIC_ExtDeviceState.DS_SCOPE != DS_SCOPE_NONE)
			{
				// SCOPE not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_READ_TIMEOUT);
			}
			else
				CONTROL_SwitchToFault(FAULT_LOGIC_GENERAL, FAULTEX_READ_TIMEOUT);
			
			LOGIC_State = LS_Error;
		}
		
		LOGIC_HandleCommunicationError();
	}
	else
		CONTROL_RequestDPC(NULL);
	
}
// ----------------------------------------

void LOGIC_TqExtraLogic(Boolean DeviceTriggered)
{
	// Main logic
	if(LOGIC_PulseNumRemain > 0)
	{
		// Detect first pulse
		if((LOGIC_PulseNumRemain + 1) == UNIT_TQ_MEASURE_PULSES)
		{
			if(DeviceTriggered)
			{
				// Init slow thyristors measurement
				CROVU_TrigTime = DC_CurrentZeroPoint + TQ_MAX_TIME;
				CROVU_TrigTime_LastHalf = TQ_MAX_TIME >> 1;
			}
			else
			{
				// Init fast thyristors measurement
				TqFastThyristor = TRUE;
				--LOGIC_PulseNumRemain;
				
				CROVU_TrigTime = DC_CurrentZeroPoint * 10 + TQ_FIRST_PROBE * 10;
				CROVU_TrigTime_LastHalf = (TQ_FIRST_PROBE * 10) >> 1;
				LOGIC_TrimTrigTime(FALSE);
			}
			
			LOGIC_PreciseEventInit(CROVU_TrigTime);
		}
		// Normal operation
		else
		{
			LOGIC_TrimTrigTime(DeviceTriggered);
			LOGIC_PreciseEventInit(CROVU_TrigTime);
		}
	}
	// Add extra iterations to trig device and then untrig
	else if((ResultsCounter + 1) < UNIT_MAX_NUM_OF_PULSES)
	{
		if(DeviceTriggered)
		{
			DUTFinalIncrease = TRUE;
			LOGIC_PulseNumRemain = 1;
			LOGIC_PreciseEventInit(++CROVU_TrigTime);
		}
		else if(!DUTFinalIncrease)
		{
			LOGIC_PulseNumRemain = 1;
			LOGIC_PreciseEventInit(--CROVU_TrigTime);
		}
	}
	
	// In case of fail
	if(LOGIC_PulseNumRemain == 0 && DeviceTriggered)
	{
		DataTable[REG_WARNING] = WARNING_DEVICE_TRIGGERED;
		LOGIC_OperationResult = OPRESULT_FAIL;
	}
}
// ----------------------------------------

void LOGIC_LogData(MeasurementResult Result)
{
	CONTROL_ValDiag1[CONTROL_ValDiag_Counter] = Result.DeviceTriggered ? 1 : 0;
	CONTROL_ValDiag2[CONTROL_ValDiag_Counter] = Result.OSVApplyTime * (TqFastThyristor ? 1 : 10);
	CONTROL_ValDiag3[CONTROL_ValDiag_Counter] = Result.Irr;
	CONTROL_ValDiag4[CONTROL_ValDiag_Counter] = Result.Trr;
	CONTROL_ValDiag5[CONTROL_ValDiag_Counter] = Result.Qrr;
	CONTROL_ValDiag6[CONTROL_ValDiag_Counter] = Result.Idc;
	CONTROL_ValDiag7[CONTROL_ValDiag_Counter] = Result.ZeroI;
	CONTROL_ValDiag8[CONTROL_ValDiag_Counter] = Result.ZeroV;
	CONTROL_ValDiag9[CONTROL_ValDiag_Counter] = Result.dIdt;
	
	++CONTROL_ValDiag_Counter;
}
// ----------------------------------------

void LOGIC_ResultToDataTable()
{
	Int16U i, AvgCounter = 0;
	Int32U AvgIrr = 0, AvgTrr = 0, AvgQrr = 0, AvgIdc = 0, AvgdIdt = 0, Irr, Trr;
	
	for(i = 0; i < ResultsCounter; ++i)
	{
		if(Results[i].Irr && Results[i].Trr && Results[i].Qrr && Results[i].Idc && Results[i].dIdt)
		{
			AvgIrr += Results[i].Irr;
			AvgTrr += Results[i].Trr;
			AvgQrr += Results[i].Qrr;
			AvgIdc += Results[i].Idc;
			AvgdIdt += Results[i].dIdt;
			
			++AvgCounter;
		}
	}
	
	// Prevent division by zero
	if(AvgCounter == 0)
		AvgCounter = 1;
	
	Trr = AvgTrr / AvgCounter;
	Irr = AvgIrr / AvgCounter;
	
	DataTable[REG_RES_QRR] = (Irr * Trr) >> 1;
	DataTable[REG_RES_TRR] = Trr;
	DataTable[REG_RES_IRR] = Irr;
	DataTable[REG_RES_IDC] = AvgIdc / AvgCounter;
	DataTable[REG_RES_TQ] = Results[ResultsCounter - 1].ZeroV - Results[ResultsCounter - 1].ZeroI;
	DataTable[REG_RES_DIDT] = AvgdIdt / AvgCounter;
	DataTable[REG_RES_QRR_INT] = (AvgQrr * 10) / AvgCounter;
	
	DataTable[REG_DC_READY_RETRIES] = LOGIC_DCReadyRetries;
}
// ----------------------------------------

void LOGIC_AbortMeasurement(Int16U WarningCode)
{
	DataTable[REG_WARNING] = WarningCode;
	LOGIC_OperationResult = OPRESULT_FAIL;
	LOGIC_Halt();
}
// ----------------------------------------

void LOGIC_SafetyProblem()
{
	DataTable[REG_PROBLEM] = PROBLEM_SAFETY;
	LOGIC_OperationResult = OPRESULT_FAIL;
	LOGIC_Halt();
}
// ----------------------------------------

void CONTROL_CSU()
{
	// Control voltage
	//
	ZwADC_StartSEQ1();
	
	if(CSUVoltage > CSU_VOLTAGE_HIGH + CSU_VOLTAGE_HYST)
	{
		ZbGPIO_CSU_PWRCtrl(FALSE);
		ZbGPIO_CSU_Disch(TRUE);
	}
	
	if(CSUVoltage < CSU_VOLTAGE_LOW - CSU_VOLTAGE_HYST)
	{
		ZbGPIO_CSU_PWRCtrl(TRUE);
		ZbGPIO_CSU_Disch(FALSE);
	}
	
	if((CSUVoltage <= CSU_VOLTAGE_HIGH) || (CSUVoltage >= CSU_VOLTAGE_LOW))
	{
		ZbGPIO_CSU_PWRCtrl(FALSE);
		ZbGPIO_CSU_Disch(FALSE);
	}
	//
	
	// Control FAN
	//
	if(LOGIC_StateRealTime == LSRT_DirectPulseStart)
	{
		ZbGPIO_CSU_FAN(TRUE);
		CSU_FanTimeout = CONTROL_TimeCounter;
	}
	
	if(CONTROL_TimeCounter > CSU_FanTimeout + CSU_FAN_TIMEOUT)
		ZbGPIO_CSU_FAN(FALSE);
	//
}
// ----------------------------------------

void CSU_VoltageMeasuring(Int16U * const restrict pResults)
{
	CSUVoltage = *(Int16U *)pResults /* (float)DataTable[REG_CSU_V_C]/1000;*/;
}
// ----------------------------------------

