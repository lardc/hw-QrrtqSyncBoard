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
static Int16U DC_Current, DC_CurrentRiseRate, DC_CurrentFallRate, DC_CurrentPlateTicks, DC_CurrentZeroPoint;
static Int16U CROVU_Voltage, CROVU_VoltageRate, FCROVU_IShortCircuit;
static volatile Int16U CROVU_TrigTime, CROVU_TrigTime_LastHalf;
static volatile Int16U LOGIC_PulseNumRemain, LOGIC_DCReadyRetries, LOGIC_OperationResult, LOGIC_DriverOffTicks;
static Int16U CSUVoltage = 0;
static DRCUConfig DCUConfig, RCUConfig;

// Forward functions
//
void LOGIC_PreciseEventInit(Int16U usTime);
void LOGIC_PreciseEventStart();
void LOGIC_TqExtraLogic(Boolean DeviceTriggered);
void LOGIC_PrepareDRCUConfig(Boolean Emulation1, Boolean Emulation2, Boolean Emulation3, Int16U Current, Int16U FallRate_x10,
		pDRCUConfig Config, Int16U RCUTrigOffset);
Int16U LOGIC_FindRCUTrigOffset(Int16U FallRate_x100);

// Functions
//
void LOGIC_RealTime()
{
	static Int32U TimeReverseStart = 0, TimeReverseStop = 0, TimeBeforeDataRead = 0;
	
	if(LOGIC_StateRealTime != LSRT_None && LOGIC_StateRealTime != LSRT_WaitForConfig)
	{
		// Wait for direct current ready pulse from QPU
		if(LOGIC_StateRealTime == LSRT_DirectPulseStart && (EmulateDCU1 || ZbGPIO_DCU1_Ready())
				&& (EmulateDCU2 || ZbGPIO_DCU2_Ready()) && (EmulateDCU3 || ZbGPIO_DCU3_Ready()))
		{
			TimeReverseStart = LOGIC_RealTimeCounter + DC_CurrentPlateTicks;
			LOGIC_StateRealTime = LSRT_DirectPulseReady;
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
			DSP28x_usDelay(RCUConfig.RCUTrigOffsetTicks);
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
		DC_CurrentPlateTicks = DataTable[REG_DCU_PULSE_WIDTH] / TIMER2_PERIOD;
		DC_CurrentRiseRate = DataTable[REG_DCU_I_RISE_RATE];
		DC_CurrentFallRate = DataTable[REG_CURRENT_FALL_RATE];

		// Подготовка конфигурации DCU и RCU
		Int16U SplittedFallRate = DC_CurrentFallRate * 10 / 2;
		LOGIC_PrepareDRCUConfig(EmulateDCU1, EmulateDCU2, EmulateDCU3, DC_Current, SplittedFallRate, &DCUConfig, 0);

		Int16U TrigOffset = LOGIC_FindRCUTrigOffset(SplittedFallRate);
		LOGIC_PrepareDRCUConfig(EmulateRCU1, EmulateRCU2, EmulateRCU3, DC_Current, SplittedFallRate, &RCUConfig, TrigOffset);

		DC_CurrentZeroPoint = DC_Current * 10 / DC_CurrentFallRate;
		DC_CurrentZeroPoint = (DC_CurrentZeroPoint > TQ_ZERO_OFFSET) ? (DC_CurrentZeroPoint - TQ_ZERO_OFFSET) : 0;
		
		CROVU_Voltage = DataTable[REG_OFF_STATE_VOLTAGE];
		CROVU_VoltageRate = DataTable[REG_OSV_RATE];
		
		FCROVU_IShortCircuit = DataTable[REG_FCROVU_I_SHORT];
		
		LOGIC_DriverOffTicks = (
				((DC_Current / DC_CurrentRiseRate / 2) > DC_DRIVER_OFF_DELAY_MIN) ?
						(DC_Current / DC_CurrentRiseRate / 2) : DC_DRIVER_OFF_DELAY_MIN) / TIMER2_PERIOD;
		
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
		if(HLI_RS232_Read16(COMM_REG_DEV_STATE, &Register))
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
		
		switch(LOGIC_State)
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
				if(!EmulateSCOPE && (LOGIC_ExtDeviceState.DS_SCOPE == CDS_Fault))
				{
					if(HLI_RS232_CallAction(COMM_ACT_FAULT_CLEAR))
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

	CONTROL_RequestDPC(&LOGIC_PowerOnSequence);
}
// ----------------------------------------

void LOGIC_PowerOnSequence()
{
	if(LOGIC_State == LS_PON_CROVU || LOGIC_State == LS_PON_FCROVU || LOGIC_State == LS_PON_WaitStates
			|| LOGIC_State == LS_PON_DCU1 || LOGIC_State == LS_PON_DCU2 || LOGIC_State == LS_PON_DCU3
			|| LOGIC_State == LS_PON_RCU1 || LOGIC_State == LS_PON_RCU2 || LOGIC_State == LS_PON_RCU3
			|| LOGIC_State == LS_PON_CSU || LOGIC_State == LS_PON_SCOPE)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch(LOGIC_State)
		{
			case LS_PON_CROVU:
				CMN_NodePowerOn(EmulateCROVU, REG_CROVU_NODE_ID, &LOGIC_ExtDeviceState.DS_CROVU, &LOGIC_State,
						FAULT_LOGIC_CROVU, LS_PON_FCROVU);
				break;
				
			case LS_PON_FCROVU:
				CMN_NodePowerOn(EmulateFCROVU, REG_FCROVU_NODE_ID, &LOGIC_ExtDeviceState.DS_FCROVU, &LOGIC_State,
						FAULT_LOGIC_FCROVU, LS_PON_DCU1);
				break;
				
			case LS_PON_DCU1:
				CMN_NodePowerOn(EmulateDCU1, REG_DCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU1, &LOGIC_State,
						FAULT_LOGIC_DCU1, LS_PON_DCU2);
				break;

			case LS_PON_DCU2:
				CMN_NodePowerOn(EmulateDCU2, REG_DCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU2, &LOGIC_State,
						FAULT_LOGIC_DCU2, LS_PON_DCU3);
				break;

			case LS_PON_DCU3:
				CMN_NodePowerOn(EmulateDCU3, REG_DCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU3, &LOGIC_State,
						FAULT_LOGIC_DCU3, LS_PON_RCU1);
				break;

			case LS_PON_RCU1:
				CMN_NodePowerOn(EmulateRCU1, REG_RCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU1, &LOGIC_State,
						FAULT_LOGIC_RCU1, LS_PON_RCU2);
				break;
				
			case LS_PON_RCU2:
				CMN_NodePowerOn(EmulateRCU2, REG_RCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU2, &LOGIC_State,
						FAULT_LOGIC_RCU2, LS_PON_RCU3);
				break;
				
			case LS_PON_RCU3:
				CMN_NodePowerOn(EmulateRCU3, REG_RCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU3, &LOGIC_State,
						FAULT_LOGIC_RCU3, LS_PON_CSU);
				break;

			case LS_PON_CSU:
				{
					if(!EmulateCSU)
						ZbGPIO_CSU_PWRCtrl(TRUE);
					else
						LOGIC_ExtDeviceState.DS_CSU = CDS_Ready;
					LOGIC_State = LS_PON_SCOPE;
				}
				break;
				
			case LS_PON_SCOPE:
				{
					if(!EmulateSCOPE)
					{
						switch(LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case CDS_Fault:
								if(HLI_RS232_CallAction(COMM_ACT_FAULT_CLEAR))
									LOGIC_State = LS_PON_WaitStates;
								break;
							case CDS_Disabled:
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE, FAULTEX_PON_WRONG_STATE);
								break;
							case CDS_InProcess:
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
						LOGIC_ExtDeviceState.DS_SCOPE = CDS_None;
						LOGIC_State = LS_PON_WaitStates;
					}
					
					if(LOGIC_State == LS_PON_WaitStates)
						Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC;
				}
				break;
				
			case LS_PON_WaitStates:
				CMN_WaitNodesReady(CONTROL_TimeCounter, Timeout, LOGIC_ExtDeviceState, &LOGIC_State);
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

	CONTROL_RequestDPC(&LOGIC_ConfigureSequence);
}
// ----------------------------------------

void LOGIC_ConfigureSequence()
{
	if(LOGIC_State == LS_CFG_CROVU || LOGIC_State == LS_CFG_FCROVU || LOGIC_State == LS_CFG_SCOPE
			|| LOGIC_State == LS_CFG_DCU1 || LOGIC_State == LS_CFG_DCU2 || LOGIC_State == LS_CFG_DCU3
			|| LOGIC_State == LS_CFG_RCU1 || LOGIC_State == LS_CFG_RCU2 || LOGIC_State == LS_CFG_RCU3
			|| LOGIC_State == LS_CFG_WaitStates)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch(LOGIC_State)
		{
			case LS_CFG_CROVU:
				{
					if(!EmulateCROVU && !MuteCROVU)
					{
						if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_ENABLE_EXT_SYNC))
							if(HLI_CAN_Write16(DataTable[REG_CROVU_NODE_ID], REG_CROVU_DESIRED_VOLTAGE, CROVU_Voltage))
								if(HLI_CAN_Write16(DataTable[REG_CROVU_NODE_ID], REG_CROVU_VOLTAGE_RATE,
										CROVU_VoltageRate))
									if(HLI_CAN_CallAction(DataTable[REG_CROVU_NODE_ID], ACT_CROVU_APPLY_SETTINGS))
										LOGIC_State = LS_CFG_FCROVU;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_CROVU = CDS_Ready;
						LOGIC_State = LS_CFG_FCROVU;
					}
				}
				break;
				
			case LS_CFG_FCROVU:
				{
					if(!EmulateFCROVU && !MuteCROVU)
					{
						if(HLI_CAN_Write16(DataTable[REG_FCROVU_NODE_ID], REG_FCROVU_V_RATE_VALUE, CROVU_VoltageRate))
							if(HLI_CAN_Write16(DataTable[REG_FCROVU_NODE_ID], REG_FCROVU_I_SHORT_CIRCUIT,
									FCROVU_IShortCircuit))
								if(HLI_CAN_CallAction(DataTable[REG_FCROVU_NODE_ID], ACT_FCROVU_CONFIG))
									LOGIC_State = LS_CFG_DCU1;
					}
					else
					{
						LOGIC_ExtDeviceState.DS_FCROVU = CDS_Ready;
						LOGIC_State = LS_CFG_DCU1;
					}
				}
				break;
				
			case LS_CFG_DCU1:
				CMN_ConfigDRCU(EmulateDCU1, REG_DCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU1, &DCUConfig, &LOGIC_State,
						LS_CFG_DCU2);
				break;

			case LS_CFG_DCU2:
				CMN_ConfigDRCU(EmulateDCU2, REG_DCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU2, &DCUConfig, &LOGIC_State,
						LS_CFG_DCU3);
				break;

			case LS_CFG_DCU3:
				CMN_ConfigDRCU(EmulateDCU3, REG_DCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU3, &DCUConfig, &LOGIC_State,
						LS_CFG_RCU1);
				break;

			case LS_CFG_RCU1:
				CMN_ConfigDRCU(EmulateRCU1, REG_RCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU1, &RCUConfig, &LOGIC_State,
						LS_CFG_RCU2);
				break;

			case LS_CFG_RCU2:
				CMN_ConfigDRCU(EmulateRCU2, REG_RCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU2, &RCUConfig, &LOGIC_State,
						LS_CFG_RCU3);
				break;
				
			case LS_CFG_RCU3:
				CMN_ConfigDRCU(EmulateRCU3, REG_RCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU3, &RCUConfig, &LOGIC_State,
						LS_CFG_SCOPE);
				break;
				
			case LS_CFG_SCOPE:
				{
					if(!EmulateSCOPE)
					{
						switch(LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case CDS_None:
								{
									if(HLI_RS232_Write16(REG_SCOPE_CURRENT_AMPL, DataTable[REG_DIRECT_CURRENT]))
										if(HLI_RS232_Write16(REG_SCOPE_MEASURE_MODE, MeasurementMode))
											if(HLI_RS232_Write16(REG_SCOPE_TR_050_METHOD,
													DataTable[REG_TRR_DETECTION_MODE]))
												if(HLI_RS232_CallAction(ACT_SCOPE_START_TEST))
													LOGIC_State = LS_CFG_WaitStates;
								}
								break;
							case CDS_InProcess:
								HLI_RS232_CallAction(ACT_SCOPE_STOP_TEST);
								break;
						}
					}
					else
					{
						LOGIC_ExtDeviceState.DS_SCOPE = CDS_InProcess;
						LOGIC_State = LS_CFG_WaitStates;
					}
					
					if(LOGIC_State == LS_CFG_WaitStates)
						Timeout = CONTROL_TimeCounter + TIMEOUT_HL_LOGIC;
				}
				break;
				
			case LS_CFG_WaitStates:
				CMN_WaitNodesReady(CONTROL_TimeCounter, Timeout, LOGIC_ExtDeviceState, &LOGIC_State);
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
	if(LOGIC_State == LS_POFF_CROVU || LOGIC_State == LS_POFF_FCROVU || LOGIC_State == LS_POFF_SCOPE
			|| LOGIC_State == LS_POFF_DCU1 || LOGIC_State == LS_POFF_DCU2 || LOGIC_State == LS_POFF_DCU3
			|| LOGIC_State == LS_POFF_RCU1 || LOGIC_State == LS_POFF_RCU2 || LOGIC_State == LS_POFF_RCU3)
	{
		if(!LOGIC_UpdateDeviceState())
		{
			LOGIC_HandleCommunicationError();
			return;
		}
		
		switch(LOGIC_State)
		{
			case LS_POFF_CROVU:
				CMN_NodePowerOff(EmulateCROVU, REG_CROVU_NODE_ID, &LOGIC_ExtDeviceState.DS_CROVU, &LOGIC_State,
						FAULT_LOGIC_CROVU, LS_POFF_FCROVU);
				break;
				
			case LS_POFF_FCROVU:
				CMN_NodePowerOff(EmulateFCROVU, REG_FCROVU_NODE_ID, &LOGIC_ExtDeviceState.DS_FCROVU, &LOGIC_State,
						FAULT_LOGIC_FCROVU, LS_POFF_DCU1);
				break;
				
			case LS_POFF_DCU1:
				CMN_NodePowerOff(EmulateDCU1, REG_DCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU1, &LOGIC_State,
						FAULT_LOGIC_DCU1, LS_POFF_DCU2);
				break;

			case LS_POFF_DCU2:
				CMN_NodePowerOff(EmulateDCU2, REG_DCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU2, &LOGIC_State,
						FAULT_LOGIC_DCU2, LS_POFF_DCU3);
				break;

			case LS_POFF_DCU3:
				CMN_NodePowerOff(EmulateDCU3, REG_DCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_DCU3, &LOGIC_State,
						FAULT_LOGIC_DCU3, LS_POFF_RCU1);
				break;

			case LS_POFF_RCU1:
				CMN_NodePowerOff(EmulateRCU1, REG_RCU1_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU1, &LOGIC_State,
						FAULT_LOGIC_RCU1, LS_POFF_RCU2);
				break;

			case LS_POFF_RCU2:
				CMN_NodePowerOff(EmulateRCU2, REG_RCU2_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU2, &LOGIC_State,
						FAULT_LOGIC_RCU2, LS_POFF_RCU3);
				break;
				
			case LS_POFF_RCU3:
				CMN_NodePowerOff(EmulateRCU3, REG_RCU3_NODE_ID, &LOGIC_ExtDeviceState.DS_RCU3, &LOGIC_State,
						FAULT_LOGIC_RCU3, LS_POFF_SCOPE);
				break;
				
			case LS_POFF_SCOPE:
				{
					if(!EmulateSCOPE)
					{
						switch(LOGIC_ExtDeviceState.DS_SCOPE)
						{
							case CDS_InProcess:
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
						LOGIC_ExtDeviceState.DS_SCOPE = CDS_None;
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
		
		switch(LOGIC_State)
		{
			case LS_READ_CROVU:
				{
					if(!EmulateCROVU)
					{
						if(LOGIC_ExtDeviceState.DS_CROVU == CDS_Ready)
							if(HLI_CAN_Read16(DataTable[REG_CROVU_NODE_ID], COMM_REG_OP_RESULT, &Register))
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
						if(LOGIC_ExtDeviceState.DS_FCROVU == CDS_Ready)
							if(HLI_CAN_Read16(DataTable[REG_FCROVU_NODE_ID], COMM_REG_OP_RESULT, &Register))
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
					Int16U Idc1Temp = 0, Idc2Temp = 0, Idc3Temp = 0;

					if(CMN_ReadDRCUCurrent(EmulateDCU1, REG_DCU1_NODE_ID, LOGIC_ExtDeviceState.DS_DCU1, &Idc1Temp))
						if(CMN_ReadDRCUCurrent(EmulateDCU2, REG_DCU2_NODE_ID, LOGIC_ExtDeviceState.DS_DCU2, &Idc2Temp))
							if(CMN_ReadDRCUCurrent(EmulateDCU3, REG_DCU3_NODE_ID, LOGIC_ExtDeviceState.DS_DCU3, &Idc3Temp))
							{
								LOGIC_State = LS_READ_SCOPE;
								Results[ResultsCounter].Idc = Idc1Temp + Idc2Temp + Idc3Temp;
							}
				}
				break;
				
			case LS_READ_SCOPE:
				{
					if(!EmulateSCOPE)
					{
						Int16U Problem;
						Boolean Result = TRUE;
						
						if(LOGIC_ExtDeviceState.DS_SCOPE == CDS_None)
						{
							if(Result) Result &= HLI_RS232_Read16(COMM_REG_OP_RESULT, &Register);
							if(Result) Result &= HLI_RS232_Read16(COMM_REG_PROBLEM, &Problem);
							if(Result) Result &= HLI_RS232_Read16(REG_SCOPE_RESULT_IRR, &Results[ResultsCounter].Irr);
							if(Result) Result &= HLI_RS232_Read16(REG_SCOPE_RESULT_TRR, &Results[ResultsCounter].Trr);
							if(Result) Result &= HLI_RS232_Read16(REG_SCOPE_RESULT_QRR, &Results[ResultsCounter].Qrr);
							if(Result) Result &= HLI_RS232_Read16(REG_SCOPE_RESULT_ZERO, &Results[ResultsCounter].ZeroI);
							if(Result) Result &= HLI_RS232_Read16(REG_SCOPE_RESULT_ZERO_V, &Results[ResultsCounter].ZeroV);
							if(Result) Result &= HLI_RS232_Read16(REG_SCOPE_RESULT_DIDT, &Results[ResultsCounter].dIdt);

							if(Result && Register == OPRESULT_NONE)
							{
								LOGIC_State = LS_Error;
								CONTROL_SwitchToFault(FAULT_LOGIC_SCOPE,
								FAULTEX_READ_WRONG_STATE);
							}
							else if((Register == OPRESULT_FAIL && Problem != PROBLEM_SCOPE_CALC_VZ)
									|| (Register == OPRESULT_FAIL && Problem == PROBLEM_SCOPE_CALC_VZ
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
									LOGIC_TqExtraLogic(Results[ResultsCounter].DeviceTriggered);

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
		
		CMN_WaitNodesReady(CONTROL_TimeCounter, Timeout, LOGIC_ExtDeviceState, &LOGIC_State);
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
	
	// Control FAN
	if(LOGIC_StateRealTime == LSRT_DirectPulseStart)
	{
		ZbGPIO_CSU_FAN(TRUE);
		CSU_FanTimeout = CONTROL_TimeCounter;
	}
	
	if(CONTROL_TimeCounter > CSU_FanTimeout + CSU_FAN_TIMEOUT)
		ZbGPIO_CSU_FAN(FALSE);
}
// ----------------------------------------

void CSU_VoltageMeasuring(Int16U * const restrict pResults)
{
	CSUVoltage = *(Int16U *)pResults;
}
// ----------------------------------------

void LOGIC_PrepareDRCUConfig(Boolean Emulation1, Boolean Emulation2, Boolean Emulation3, Int16U Current, Int16U FallRate_x100,
		pDRCUConfig Config, Int16U RCUTrigOffset)
{
	Int16U BlockCounter = 0;

	BlockCounter += Emulation1 ? 1 : 0;
	BlockCounter += Emulation2 ? 1 : 0;
	BlockCounter += Emulation3 ? 1 : 0;

	if(BlockCounter)
	{
		Config->Current_x10 = Current * 10 / BlockCounter;
		Config->CurrentRate_x100 = FallRate_x100 / BlockCounter;

		Int32S Ticks = ((Int32S)RCUTrigOffset * CPU_FRQ_MHZ / 1000 - 9) / 5;
		Config->RCUTrigOffsetTicks = (Ticks > 0) ? Ticks : 0;
	}
}
// ----------------------------------------

Int16U LOGIC_FindRCUTrigOffset(Int16U FallRate_x100)
{
	switch(FallRate_x100)
	{
		case 50:
			return DataTable[REG_RCU_TOFFS_R050];

		case 75:
			return DataTable[REG_RCU_TOFFS_R075];

		case 100:
			return DataTable[REG_RCU_TOFFS_R100];

		case 250:
			return DataTable[REG_RCU_TOFFS_R250];

		case 500:
			return DataTable[REG_RCU_TOFFS_R500];

		case 750:
			return DataTable[REG_RCU_TOFFS_R750];

		case 1000:
			return DataTable[REG_RCU_TOFFS_R1000];

		case 1500:
			return DataTable[REG_RCU_TOFFS_R1500];

		case 2500:
			return DataTable[REG_RCU_TOFFS_R2500];

		case 3000:
			return DataTable[REG_RCU_TOFFS_R3000];

		case 5000:
			return DataTable[REG_RCU_TOFFS_R5000];

		default:
			return DataTable[REG_RCU_TOFFS_R1000];
	}
}
// ----------------------------------------
