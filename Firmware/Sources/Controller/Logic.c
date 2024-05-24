// -----------------------------------------
// Main logic library
// ----------------------------------------

// Header
#include "Logic.h"
//
// Includes
#include "xCCICommon.h"
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
	Int16U DS_FCROVU;
	Int16U DS_QPU;
	Int16U DS_SCOPE;
} ExternalDeviceState;

// Variables
//
volatile DeviceSubState LOGIC_StateRealTime = LSRT_None;
volatile Int32U LOGIC_RealTimeCounter = 0;
static volatile Int64U Timeout;
static volatile LogicState LOGIC_State = LS_None;
static volatile ExternalDeviceState LOGIC_ExtDeviceState;
//
static MeasurementResult Results[UNIT_MAX_NUM_OF_PULSES];
static volatile Int16U ResultsCounter, MeasurementMode;
//
static Boolean EmulateFCROVU, EmulateQPU, EmulateSCOPE, MuteFCROVU;
static Boolean CacheUpdate = FALSE, CacheSinglePulse = FALSE, DCPulseFormed = FALSE;
static volatile Boolean TqFastThyristor = FALSE, DUTFinalIncrease = FALSE;
static Int16U QPU_Current, QPU_CurrentEdgeTime, QPU_CurrentFallRate, QPU_CurrentPlateTicks, QPU_CurrentZeroPoint;
static Int16U FCROVU_Voltage, FCROVU_VoltageRate;
static volatile Int16U FCROVU_TrigTime, FCROVU_TrigTime_LastHalf;
static volatile Int16U LOGIC_PulseNumRemain, LOGIC_DCReadyRetries, LOGIC_OperationResult, LOGIC_DriverOffTicks;

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

	if (LOGIC_StateRealTime != LSRT_None && LOGIC_StateRealTime != LSRT_WaitForConfig)
	{
		// Wait for direct current ready pulse from QPU
		if (LOGIC_StateRealTime == LSRT_DirectPulseStart && ZbGPIO_DirectCurrentReady())
		{
			TimeReverseStart = LOGIC_RealTimeCounter + QPU_CurrentPlateTicks;
			LOGIC_StateRealTime = LSRT_DirectPulseReady;
		}

		// Turn off gate driver
		if (LOGIC_RealTimeCounter >= LOGIC_DriverOffTicks)
			ZbGPIO_SwitchDUT(FALSE);

		// In case of direct current ready pulse timeout
		if (LOGIC_StateRealTime == LSRT_DirectPulseStart && LOGIC_RealTimeCounter > DC_READY_TIMEOUT_TICK)
		{
			ZbGPIO_SwitchDirectCurrent(FALSE);
			ZbGPIO_SwitchHVIGBT(FALSE);
			LOGIC_StateRealTime = LSRT_None;

			if (++LOGIC_DCReadyRetries > DC_READY_RETRIES_NUM)
			{
				DataTable[REG_DC_READY_RETRIES] = LOGIC_DCReadyRetries;
				LOGIC_AbortMeasurement(WARNING_NO_DIRECT_CURRENT);
			}
		}

		// Start reverse current pulse and on-state voltage timer
		if (LOGIC_StateRealTime == LSRT_DirectPulseReady && LOGIC_RealTimeCounter > TimeReverseStart)
		{
			ZbGPIO_SyncSCOPE(TRUE);
			ZbGPIO_SwitchReverseCurrent(TRUE);
			ZbGPIO_SwitchDirectCurrent(FALSE);
			ZbGPIO_SwitchDUT(FALSE);

			LOGIC_StateRealTime = LSRT_ReversePulseStart;
			TimeReverseStop = LOGIC_RealTimeCounter + OSV_ON_TIME_TICK;
			LOGIC_PreciseEventStart();
		}

		// Stop process
		if (LOGIC_StateRealTime == LSRT_ReversePulseStart && LOGIC_RealTimeCounter >= TimeReverseStop)
		{
			ZbGPIO_SyncFCROVU(FALSE);
			ZbGPIO_SyncSCOPE(FALSE);
			//
			ZbGPIO_SwitchReverseCurrent(FALSE);
			ZbGPIO_SwitchHVIGBT(FALSE);

			TimeBeforeDataRead = LOGIC_RealTimeCounter + RT_DATA_READ_DELAY_TICK;
			LOGIC_StateRealTime = LSRT_ReadDataPause;
		}

		if (LOGIC_StateRealTime == LSRT_ReadDataPause && LOGIC_RealTimeCounter >= TimeBeforeDataRead)
		{
			Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC_SHORT;

			if (LOGIC_PulseNumRemain > 0)
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

	if (LOGIC_StateRealTime == LSRT_WaitForConfig)
		LOGIC_StateRealTime = LSRT_None;

	LOGIC_State = LS_None;
}
// ----------------------------------------

void LOGIC_TrimTrigTime(Boolean Increase)
{
	if (Increase)
		FCROVU_TrigTime += FCROVU_TrigTime_LastHalf;
	else
		FCROVU_TrigTime -= FCROVU_TrigTime_LastHalf;

	FCROVU_TrigTime_LastHalf >>= 1;
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
	if (err.ErrorCode != ERR_NO_ERROR)
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
	EmulateFCROVU = DataTable[REG_EMULATE_FCROVU] ? TRUE : FALSE;
	EmulateQPU = DataTable[REG_EMULATE_QPU] ? TRUE : FALSE;
	EmulateSCOPE = DataTable[REG_EMULATE_SCOPE] ? TRUE : FALSE;

	if (CacheUpdate)
	{
		TqFastThyristor = FALSE;
		DUTFinalIncrease = FALSE;
		LOGIC_OperationResult = OPRESULT_OK;

		LOGIC_DCReadyRetries = 0;
		ResultsCounter = 0;
		MeasurementMode = DataTable[REG_MODE];
		MuteFCROVU = (MeasurementMode == MODE_QRR_ONLY) ? TRUE : FALSE;

		QPU_Current = DataTable[REG_DIRECT_CURRENT];
		QPU_CurrentEdgeTime = (DataTable[REG_DIRECT_CURRENT] * 10) / DataTable[REG_DC_RISE_RATE] + 1;
		QPU_CurrentPlateTicks = DataTable[REG_DC_PULSE_WIDTH] / TIMER2_PERIOD;
		QPU_CurrentFallRate = DataTable[REG_DC_FALL_RATE];

		QPU_CurrentZeroPoint = QPU_Current * 10 / QPU_CurrentFallRate;
		QPU_CurrentZeroPoint = (QPU_CurrentZeroPoint > TQ_ZERO_OFFSET) ? (QPU_CurrentZeroPoint - TQ_ZERO_OFFSET) : 0;

		// Грязный патч для работы на напряжениях выше 1798В (по 899В на ячейку)
		FCROVU_Voltage = (DataTable[REG_OFF_STATE_VOLTAGE] > 1798) ? 1798 : DataTable[REG_OFF_STATE_VOLTAGE];
		FCROVU_VoltageRate = DataTable[REG_OSV_RATE];

		LOGIC_DriverOffTicks = (((QPU_CurrentEdgeTime / 2) > DC_DRIVER_OFF_DELAY_MIN) ?
									(QPU_CurrentEdgeTime / 2) : DC_DRIVER_OFF_DELAY_MIN) / TIMER2_PERIOD;

		if (MeasurementMode == MODE_QRR_ONLY)
		{
			LOGIC_PulseNumRemain = QRR_AVG_COUNTER;
			FCROVU_TrigTime = QPU_CurrentZeroPoint + TQ_MAX_TIME;
		}
		else if (CacheSinglePulse)
		{
			LOGIC_PulseNumRemain = 1;
			FCROVU_TrigTime = QPU_CurrentZeroPoint + DataTable[REG_TRIG_TIME];
		}
		else
		{
			LOGIC_PulseNumRemain = UNIT_TQ_MEASURE_PULSES;
			FCROVU_TrigTime = QPU_CurrentZeroPoint + TQ_FIRST_PROBE;
		}

		LOGIC_PreciseEventInit(FCROVU_TrigTime);
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

	if (!EmulateFCROVU)
	{
		if (HLI_CAN_Read16(NODEID_FCROVU, REG_FCROVU_DEV_STATE, &Register))
			LOGIC_ExtDeviceState.DS_FCROVU = Register;
		else
			return FALSE;
	}

	if (!EmulateQPU)
	{
		if (HLI_CAN_Read16(NODEID_QPU, REG_QPU_DEV_STATE, &Register))
			LOGIC_ExtDeviceState.DS_QPU = Register;
		else
			return FALSE;
	}

	if (!EmulateSCOPE)
	{
		if (HLI_RS232_Read16(REG_SCOPE_DEV_STATE, &Register))
			LOGIC_ExtDeviceState.DS_SCOPE = Register;
		else
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

void LOGIC_FaultResetPrepare()
{
	LOGIC_CacheVariables();
	LOGIC_State = LS_CLR_FCROVU;
	//
	CONTROL_RequestDPC(&LOGIC_FaultResetSequence);
}
// ----------------------------------------

void LOGIC_FaultResetSequence()
{
	if (LOGIC_State == LS_CLR_FCROVU || LOGIC_State == LS_CLR_QPU || LOGIC_State == LS_CLR_SCOPE)
	{
		if (!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}

		switch (LOGIC_State)
		{
			case LS_CLR_FCROVU:
				{
					if (!EmulateFCROVU && (LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_FAULT))
					{
						if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_CLR_FAULT))
							LOGIC_State = LS_CLR_QPU;
					}
					else
						LOGIC_State = LS_CLR_QPU;
				}
				break;

			case LS_CLR_QPU:
				{
					if (!EmulateQPU && (LOGIC_ExtDeviceState.DS_QPU == DS_QPU_FAULT))
					{
						if (HLI_CAN_CallAction(NODEID_QPU, ACT_QPU_CLR_FAULT))
							LOGIC_State = LS_CLR_SCOPE;
					}
					else
						LOGIC_State = LS_CLR_SCOPE;
				}
				break;

			case LS_CLR_SCOPE:
				{
					if (!EmulateSCOPE && (LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_FAULT))
					{
						if (HLI_RS232_CallAction(ACT_SCOPE_CLR_FAULT))
							LOGIC_State = LS_None;
					}
					else
						LOGIC_State = LS_None;
				}
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
	LOGIC_State = LS_PON_FCROVU;
	//
	CONTROL_RequestDPC(&LOGIC_PowerOnSequence);
}
// ----------------------------------------

void LOGIC_PowerOnSequence()
{
	if 	(LOGIC_State == LS_PON_FCROVU || LOGIC_State == LS_PON_QPU ||
		 LOGIC_State == LS_PON_SCOPE || LOGIC_State == LS_PON_WaitStates)
	{
		if (!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}

		switch (LOGIC_State)
		{
			case LS_PON_FCROVU:
				{
					// Handle FCROVU node
					if (!EmulateFCROVU)
					{
						switch (LOGIC_ExtDeviceState.DS_FCROVU)
						{
							case DS_FCROVU_NONE:
								if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_ENABLE_POWER))
									LOGIC_State = LS_PON_QPU;
								break;
							case DS_FCROVU_READY:
								if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_DISABLE_POWER))
									if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_ENABLE_POWER))
										LOGIC_State = LS_PON_QPU;
								break;
							case DS_FCROVU_FAULT:
								if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_CLR_FAULT))
									if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_ENABLE_POWER))
										LOGIC_State = LS_PON_QPU;
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
						LOGIC_State = LS_PON_QPU;
					}
				}
				break;

			case LS_PON_QPU:
				{
					if (!EmulateQPU)
					{
						switch (LOGIC_ExtDeviceState.DS_QPU)
						{
							case DS_QPU_NONE:
								if (HLI_CAN_CallAction(NODEID_QPU, ACT_QPU_ENABLE_POWER))
									LOGIC_State = LS_PON_SCOPE;
								break;
							case DS_QPU_FAULT:
								if (HLI_CAN_CallAction(NODEID_QPU, ACT_QPU_CLR_FAULT))
									if (HLI_CAN_CallAction(NODEID_QPU, ACT_QPU_ENABLE_POWER))
										LOGIC_State = LS_PON_SCOPE;
								break;
							case DS_QPU_DISABLED:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_QPU, FAULTEX_PON_WRONG_STATE);
								break;
							default:
								LOGIC_State = LS_PON_SCOPE;
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_QPU = DS_QPU_READY;
						LOGIC_State = LS_PON_SCOPE;
					}
				}
				break;

			case LS_PON_SCOPE:
				{
					// Handle SCOPE node
					if (!EmulateSCOPE)
					{
						switch (LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case DS_SCOPE_FAULT:
								if (HLI_RS232_CallAction(ACT_SCOPE_CLR_FAULT))
									LOGIC_State = LS_PON_WaitStates;
								break;
							case DS_SCOPE_DISABLED:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_PON_WRONG_STATE);
								break;
							case DS_SCOPE_IN_PROCESS:
								if (HLI_RS232_CallAction(ACT_SCOPE_STOP_TEST))
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

					if (LOGIC_State != LS_PON_SCOPE)
						Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC;
				}
				break;

			case LS_PON_WaitStates:
				{
					if (Timeout > CONTROL_TimeCounter)
					{
						if (LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_READY &&
							(LOGIC_ExtDeviceState.DS_QPU == DS_QPU_READY || LOGIC_ExtDeviceState.DS_QPU == DS_QPU_CONFIG_READY || LOGIC_ExtDeviceState.DS_QPU == DS_QPU_PULSE_END) &&
							(LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_NONE))
						{
							LOGIC_State = LS_None;
						}
					}
					else
					{
						if (LOGIC_ExtDeviceState.DS_FCROVU != DS_FCROVU_READY)
						{
							// FCROVU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_PON_TIMEOUT);
						}
						else if (LOGIC_ExtDeviceState.DS_QPU != DS_QPU_READY)
						{
							// QPU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_QPU, FAULTEX_PON_TIMEOUT);
						}
						else if (LOGIC_ExtDeviceState.DS_SCOPE != DS_SCOPE_NONE)
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
	LOGIC_State = LS_CFG_FCROVU;
	//
	CONTROL_RequestDPC(&LOGIC_ConfigureSequence);
}
// ----------------------------------------

void LOGIC_ConfigureSequence()
{
	if 	(LOGIC_State == LS_CFG_FCROVU || LOGIC_State == LS_CFG_QPU ||
		 LOGIC_State == LS_CFG_SCOPE || LOGIC_State == LS_CFG_WaitStates)
	{
		if (!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}

		switch (LOGIC_State)
		{
			case LS_CFG_FCROVU:
				{
					// Handle FCROVU node
					if (!EmulateFCROVU && !MuteFCROVU)
					{
						if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_ENABLE_EXT_SYNC))
							if (HLI_CAN_Write16(NODEID_FCROVU, REG_FCROVU_DESIRED_VOLTAGE, FCROVU_Voltage))
								if (HLI_CAN_Write16(NODEID_FCROVU, REG_FCROVU_VOLTAGE_RATE, FCROVU_VoltageRate))
									if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_APPLY_SETTINGS))
										LOGIC_State = LS_CFG_QPU;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_FCROVU = DS_FCROVU_READY;
						LOGIC_State = LS_CFG_QPU;
					}
				}
				break;

			case LS_CFG_QPU:
				{
					// Handle QPU node
					if (!EmulateQPU)
					{
						if ((LOGIC_ExtDeviceState.DS_QPU == DS_QPU_PULSE_END && ResultsCounter > 0) || ResultsCounter == 0)
						{
							if (HLI_CAN_Write16(NODEID_QPU, REG_QPU_PULSE_VALUE, QPU_Current))
								if (HLI_CAN_Write16(NODEID_QPU, REG_QPU_IDC_EDGE_TIME, QPU_CurrentEdgeTime))
									if (HLI_CAN_Write16(NODEID_QPU, REG_QPU_REVERSE_I_RATE, QPU_CurrentFallRate))
										if (HLI_CAN_CallAction(NODEID_QPU, ACT_QPU_PULSE_CONFIG))
											LOGIC_State = LS_CFG_SCOPE;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_QPU = DS_QPU_CONFIG_READY;
						LOGIC_State = LS_CFG_SCOPE;
					}
				}
				break;

			case LS_CFG_SCOPE:
				{
					// Handle SCOPE node
					if (!EmulateSCOPE)
					{
						switch (LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case DS_SCOPE_NONE:
								{
									if (HLI_RS232_Write16(REG_SCOPE_CURRENT_AMPL, DataTable[REG_DIRECT_CURRENT]))
										if (HLI_RS232_Write16(REG_SCOPE_MEASURE_MODE, MeasurementMode))
											if (HLI_RS232_Write16(REG_SCOPE_TR_050_METHOD, DataTable[REG_TRR_DETECTION_MODE]))
												if (HLI_RS232_CallAction(ACT_SCOPE_START_TEST))
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

					if (LOGIC_State != LS_CFG_SCOPE)
						Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC;
				}
				break;

			case LS_CFG_WaitStates:
				{
					if (Timeout > CONTROL_TimeCounter)
					{
						if (LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_READY &&
							LOGIC_ExtDeviceState.DS_QPU == DS_QPU_CONFIG_READY &&
							LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_IN_PROCESS)
						{
							LOGIC_State = LS_None;
						}
					}
					else
					{
						if (LOGIC_ExtDeviceState.DS_FCROVU != DS_FCROVU_READY)
						{
							// FCROVU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_CFG_TIMEOUT);
						}
						else if (LOGIC_ExtDeviceState.DS_QPU != DS_QPU_CONFIG_READY)
						{
							// QPU not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_QPU, FAULTEX_CFG_TIMEOUT);
						}
						else if (LOGIC_ExtDeviceState.DS_SCOPE != DS_SCOPE_IN_PROCESS)
						{
							// SCOPE not ready
							CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_CFG_TIMEOUT);
						}
						else
							CONTROL_SwitchToFault(FAULT_LOGIC_GENERAL, FAULTEX_CFG_TIMEOUT);

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
	LOGIC_State = LS_POFF_FCROVU;
	//
	CONTROL_RequestDPC(&LOGIC_PowerOffSequence);
}
// ----------------------------------------

void LOGIC_PowerOffSequence()
{
	if 	(LOGIC_State == LS_POFF_FCROVU || LOGIC_State == LS_POFF_QPU ||
		 LOGIC_State == LS_POFF_SCOPE)
	{
		if (!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}

		switch (LOGIC_State)
		{
			case LS_POFF_FCROVU:
				{
					// Handle FCROVU node
					if (!EmulateFCROVU)
					{
						switch (LOGIC_ExtDeviceState.DS_FCROVU)
						{
							case DS_FCROVU_NONE:
							case DS_FCROVU_READY:
								if (HLI_CAN_CallAction(NODEID_FCROVU, ACT_FCROVU_DISABLE_POWER))
									LOGIC_State = LS_POFF_QPU;
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
						LOGIC_State = LS_POFF_QPU;
					}
				}
				break;

			case LS_POFF_QPU:
			{
				if (!EmulateQPU)
				{
					switch (LOGIC_ExtDeviceState.DS_QPU)
					{
						case DS_QPU_FAULT:
						case DS_QPU_DISABLED:
						case DS_QPU_PULSE_START:
						case DS_QPU_PULSE_READY:
							LOGIC_State = LS_Error;
							CONTROL_SwitchToFault(FAULT_LOGIC_QPU, FAULTEX_POFF_WRONG_STATE);
							break;
						default:
							if (HLI_CAN_CallAction(NODEID_QPU, ACT_QPU_DISABLE_POWER))
								LOGIC_State = LS_POFF_SCOPE;
							break;
					}
				}
				else
				{
					LOGIC_ExtDeviceState.DS_QPU = DS_QPU_NONE;
					LOGIC_State = LS_POFF_SCOPE;
				}
			}
				break;

			case LS_POFF_SCOPE:
				{
					// Handle SCOPE node
					if (!EmulateSCOPE)
					{
						switch (LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case DS_SCOPE_IN_PROCESS:
								if (HLI_RS232_CallAction(ACT_SCOPE_STOP_TEST))
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
	LOGIC_State = LS_READ_FCROVU;
	CONTROL_RequestDPC(&LOGIC_ReadDataSequence);
}
// ----------------------------------------

void LOGIC_ReadDataSequence()
{
	Int16U Register;

	if 	(LOGIC_State == LS_READ_FCROVU || LOGIC_State == LS_READ_QPU ||
		 LOGIC_State == LS_READ_SCOPE)
	{
		if (!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}

		switch (LOGIC_State)
		{
			case LS_READ_FCROVU:
				{
					if (!EmulateFCROVU)
					{
						if (LOGIC_ExtDeviceState.DS_FCROVU == DS_FCROVU_READY)
							if (HLI_CAN_Read16(NODEID_FCROVU, REG_FCROVU_TEST_RESULT, &Register))
							{
								if (MeasurementMode == MODE_QRR_TQ)
								{
									if (Register == OPRESULT_NONE)
									{
										LOGIC_State = LS_Error;
										CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_READ_WRONG_STATE);
									}
									else
									{
										Results[ResultsCounter].DeviceTriggered = (Register == OPRESULT_OK) ? FALSE : TRUE;
										LOGIC_State = LS_READ_QPU;
									}
								}
								else
									LOGIC_State = LS_READ_QPU;
							}
					}
					else
						LOGIC_State = LS_READ_QPU;
				}
				break;

			case LS_READ_QPU:
				{
					if (!EmulateQPU)
					{
						if (LOGIC_ExtDeviceState.DS_QPU == DS_QPU_PULSE_END)
							if (HLI_CAN_Read16(NODEID_QPU, REG_QPU_ACTUAL_IDC, &Results[ResultsCounter].Idc))
								LOGIC_State = LS_READ_SCOPE;
					}
					else
						LOGIC_State = LS_READ_SCOPE;
				}
				break;

			case LS_READ_SCOPE:
				{
					if (!EmulateSCOPE)
					{
						Int16U Problem;

						if (LOGIC_ExtDeviceState.DS_SCOPE == DS_SCOPE_NONE)
							if (HLI_RS232_Read16(REG_SCOPE_FINISHED, &Register))
								if (HLI_RS232_Read16(REG_SCOPE_PROBLEM, &Problem))
									if (HLI_RS232_Read16(REG_SCOPE_RESULT_IRR, &Results[ResultsCounter].Irr))
										if (HLI_RS232_Read16(REG_SCOPE_RESULT_TRR, &Results[ResultsCounter].Trr))
											if (HLI_RS232_Read16(REG_SCOPE_RESULT_QRR, &Results[ResultsCounter].Qrr))
												if (HLI_RS232_Read16(REG_SCOPE_RESULT_ZERO, &Results[ResultsCounter].ZeroI))
													if (HLI_RS232_Read16(REG_SCOPE_RESULT_ZERO_V, &Results[ResultsCounter].ZeroV))
														if (HLI_RS232_Read16(REG_SCOPE_RESULT_DIDT, &Results[ResultsCounter].dIdt))
															if (HLI_RS232_Read16(REG_SCOPE_EP_ELEMENT_FRACT, &Results[ResultsCounter].EPTimeFract))
																if (HLI_RS232_Read16(REG_SCOPE_EP_STEP_FRACT_CNT, &Results[ResultsCounter].EPTimeFractCnt))
																{
																	if (Register == OPRESULT_NONE)
																	{
																		LOGIC_State = LS_Error;
																		CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_READ_WRONG_STATE);
																	}
																	else if ((Register == OPRESULT_FAIL && Problem != PROBLEM_SCOPE_CALC_VZ) ||
																			(Register == OPRESULT_FAIL && Problem == PROBLEM_SCOPE_CALC_VZ && !Results[ResultsCounter].DeviceTriggered))
																	{
																		LOGIC_AbortMeasurement(WARNING_SCOPE_CALC_FAILED);
																	}
																	else if (Results[ResultsCounter].Irr > QPU_Current)
																	{
																		LOGIC_AbortMeasurement(WARNING_IRR_TO_HIGH);
																	}
																	else
																	{
																		// Save results
																		Results[ResultsCounter].OSVApplyTime = FCROVU_TrigTime;
																		LOGIC_LogData(Results[ResultsCounter]);

																		// Apply extended Tq logic
																		if (MeasurementMode == MODE_QRR_TQ && !CacheSinglePulse)
																			LOGIC_TqExtraLogic(Results[ResultsCounter].DeviceTriggered);

																		// Read data plots
																		if (LOGIC_PulseNumRemain == 0 && DataTable[REG_DIAG_DISABLE_PLOT_READ] == 0)
																		{
																			if (HLI_RS232_ReadArray16(EP_QPU_READ_I, CONTROL_Values_1, VALUES_x_SIZE, (pInt16U)&CONTROL_Values_1_Counter))
																				if (MeasurementMode == MODE_QRR_TQ)
																				{
																					if (HLI_RS232_ReadArray16(EP_QPU_READ_V, CONTROL_Values_2, VALUES_x_SIZE, (pInt16U)&CONTROL_Values_2_Counter))
																						LOGIC_State = LS_None;
																				}
																				else
																					LOGIC_State = LS_None;
																		}
																		else
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

		if (CONTROL_TimeCounter > Timeout && LOGIC_State != LS_None)
		{
			if (LOGIC_ExtDeviceState.DS_FCROVU != DS_FCROVU_READY)
			{
				// FCROVU not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_FCROVU, FAULTEX_READ_TIMEOUT);
			}
			else if (LOGIC_ExtDeviceState.DS_QPU != DS_QPU_PULSE_END)
			{
				// QPU not ready
				CONTROL_SwitchToFault(FAULT_LOGIC_QPU, FAULTEX_READ_TIMEOUT);
			}
			else if (LOGIC_ExtDeviceState.DS_SCOPE != DS_SCOPE_NONE)
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
	if (LOGIC_PulseNumRemain > 0)
	{
		// Detect first pulse
		if ((LOGIC_PulseNumRemain + 1) == UNIT_TQ_MEASURE_PULSES)
		{
			if (DeviceTriggered)
			{
				// Init slow thyristors measurement
				FCROVU_TrigTime = QPU_CurrentZeroPoint + TQ_MAX_TIME;
				FCROVU_TrigTime_LastHalf = TQ_MAX_TIME >> 1;
			}
			else
			{
				// Init fast thyristors measurement
				TqFastThyristor = TRUE;
				--LOGIC_PulseNumRemain;

				FCROVU_TrigTime = QPU_CurrentZeroPoint * 10 + TQ_FIRST_PROBE * 10;
				FCROVU_TrigTime_LastHalf = (TQ_FIRST_PROBE * 10) >> 1;
				LOGIC_TrimTrigTime(FALSE);
			}

			LOGIC_PreciseEventInit(FCROVU_TrigTime);
		}
		// Normal operation
		else
		{
			LOGIC_TrimTrigTime(DeviceTriggered);
			LOGIC_PreciseEventInit(FCROVU_TrigTime);
		}
	}
	// Add extra iterations to trig device and then untrig
	else if ((ResultsCounter + 1) < UNIT_MAX_NUM_OF_PULSES)
	{
		if (DeviceTriggered)
		{
			DUTFinalIncrease = TRUE;
			LOGIC_PulseNumRemain = 1;
			LOGIC_PreciseEventInit(++FCROVU_TrigTime);
		}
		else if (!DUTFinalIncrease)
		{
			LOGIC_PulseNumRemain = 1;
			LOGIC_PreciseEventInit(--FCROVU_TrigTime);
		}
	}

	// In case of fail
	if (LOGIC_PulseNumRemain == 0 && DeviceTriggered)
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

	for (i = 0; i < ResultsCounter; ++i)
	{
		if (Results[i].Irr && Results[i].Trr && Results[i].Qrr && Results[i].Idc && Results[i].dIdt)
		{
			AvgIrr	+= Results[i].Irr;
			AvgTrr	+= Results[i].Trr;
			AvgQrr	+= Results[i].Qrr;
			AvgIdc	+= Results[i].Idc;
			AvgdIdt	+= Results[i].dIdt;

			++AvgCounter;
		}
	}

	// Prevent division by zero
	if (AvgCounter == 0) AvgCounter = 1;

	Trr = AvgTrr / AvgCounter;
	Irr = AvgIrr / AvgCounter;

	DataTable[REG_RES_QRR]		= (Irr * Trr) >> 1;
	DataTable[REG_RES_TRR]		= Trr;
	DataTable[REG_RES_IRR]		= Irr;
	DataTable[REG_RES_IDC]		= AvgIdc / AvgCounter;
	DataTable[REG_RES_TQ]		= Results[ResultsCounter - 1].ZeroV - Results[ResultsCounter - 1].ZeroI;
	DataTable[REG_RES_DIDT]		= AvgdIdt / AvgCounter;
	DataTable[REG_RES_QRR_INT]	= (AvgQrr * 10) / AvgCounter;

	DataTable[REG_EP_ELEMENT_FRACT] = Results[ResultsCounter - 1].EPTimeFract;
	DataTable[REG_EP_STEP_FRACT_CNT] = Results[ResultsCounter - 1].EPTimeFractCnt;

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
