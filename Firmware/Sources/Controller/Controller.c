// -----------------------------------------
// Controller logic
// ----------------------------------------

// Header
#include "Controller.h"
//
// Includes
#include "SysConfig.h"
//
#include "ZbBoard.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "SCCISlave.h"
#include "DeviceProfile.h"
#include "Constraints.h"
#include "HighLevelInterface.h"
#include "Logic.h"
#include "CommonDictionary.h"

// Definitions
//
#define PRESSURE_FAULT_COUNTER_MAX			1000

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile Int64U CONTROL_TimeCounter = 0, CONTROL_PulseToPulsePause, CONTROL_CommutationDelay;
static volatile Boolean CycleActive = FALSE, ReinitRS232 = FALSE, CommutationForcedOn = FALSE;
static volatile FUNC_AsyncDelegate DPCDelegate = NULL;
//
#pragma DATA_SECTION(CONTROL_Values_1, "data_mem");
Int16U CONTROL_Values_1[VALUES_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_2, "data_mem");
Int16U CONTROL_Values_2[VALUES_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_Slave, "data_mem");
Int16U CONTROL_Values_Slave[VALUES_x_SIZE];
volatile Int16U CONTROL_Values_1_Counter = 0, CONTROL_Values_2_Counter = 0, CONTROL_Values_Slave_Counter = 0;
//
Int16U CONTROL_ValDiag1[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag2[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag3[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag4[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag5[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag6[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag7[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag8[UNIT_MAX_NUM_OF_PULSES];
Int16U CONTROL_ValDiag9[UNIT_MAX_NUM_OF_PULSES];
volatile Int16U CONTROL_ValDiag_Counter = 0;
//
// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;

// Forward functions
//
void CONTROL_SetDeviceState(DeviceState NewState);
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
void CONTROL_StatusHLI();
void CONTROL_FillWPPartDefault();
void CONTROL_SwitchToReverseCurrent();
void CONTROL_SubProcessStateMachine();
void CONTROL_ReinitRS232();
void CONTROL_SwitchToReady();
void CONTROL_Commutation(Boolean State);
void CONTROL_SafetyHandler();
void CONTROL_PressureHandler();

// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	// Variables for endpoint configuration
	Int16U EPIndexes[EP_COUNT] = {EP_Current, EP_Voltage,
	EP_DIAG1_DevTrig, EP_DIAG2_OSVTime, EP_DIAG3_Irr, EP_DIAG4_Trr,
	EP_DIAG5_Qrr, EP_DIAG6_Idc, EP_DIAG7_ZeroI, EP_DIAG8_ZeroV, EP_DIAG9_dIdt, EP_SlaveData};
	
	Int16U EPSized[EP_COUNT] = {VALUES_x_SIZE, VALUES_x_SIZE,
	UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES,
	UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES,
			UNIT_MAX_NUM_OF_PULSES, VALUES_x_SIZE};
	
	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_Values_1_Counter, (pInt16U)&CONTROL_Values_2_Counter,
			(pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter,
			(pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter,
			(pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter,
			(pInt16U)&CONTROL_Values_Slave_Counter};
	
	pInt16U EPDatas[EP_COUNT] = {CONTROL_Values_1, CONTROL_Values_2, CONTROL_ValDiag1, CONTROL_ValDiag2,
			CONTROL_ValDiag3, CONTROL_ValDiag4, CONTROL_ValDiag5, CONTROL_ValDiag6, CONTROL_ValDiag7, CONTROL_ValDiag8,
			CONTROL_ValDiag9, CONTROL_Values_Slave};
	
	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {&ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM};
	
	// Init data table
	DT_Init(EPROMService, BadClockDetected);
	DT_SaveFirmwareInfo(DEVICE_CAN_ADDRESS, BCCI_MASTER_ADDRESS);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();
	
	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Reset control values
	DEVPROFILE_ResetControlSection();
	
	if(!BadClockDetected)
	{
		if(ZwSystem_GetDogAlarmFlag())
		{
			DataTable[REG_WARNING] = WARNING_WATCHDOG_RESET;
			ZwSystem_ClearDogAlarmFlag();
		}
	}
	else
	{
		DataTable[REG_DISABLE_REASON] = DISABLE_BAD_CLOCK;
		CONTROL_SetDeviceState(DS_Disabled);
	}
	
	ZwADC_SubscribeToResults1(&CSU_VoltageMeasuring);
}
// ----------------------------------------

void CONTROL_RequestDPC(FUNC_AsyncDelegate Action)
{
	DPCDelegate = Action;
}
// ----------------------------------------

void CONTROL_Idle()
{
	// Handle RS232 to PC interface re-init
	CONTROL_ReinitRS232();
	
	// Process external interface requests
	DEVPROFILE_ProcessRequests();
	
	// Update CAN bus status
	DEVPROFILE_UpdateCANDiagStatus();
	
	// Check high-level interface status
	CONTROL_StatusHLI();
	
	// Check long-execution process
	CONTROL_SubProcessStateMachine();
	
	// Control CSU
	CONTROL_CSU();
	
	// Update high-level logic state
	DataTable[REG_LOGIC_STATE] = LOGIC_GetState();
	
	// Process deferred procedures
	FUNC_AsyncDelegate DPCDelegateCopy = DPCDelegate;
	if(DPCDelegateCopy)
		DPCDelegateCopy();
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_Update, "ramfuncs");
#endif
void CONTROL_Update()
{
	// Handle CAN-master interface requests
	BCCIM_Process(&DEVICE_CAN_Master_Interface);

	// Process real-time tasks
	LOGIC_RealTime();
	
	CONTROL_SafetyHandler();
	CONTROL_PressureHandler();
}
// ----------------------------------------

void CONTROL_NotifyCANaFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANaFault(Flag);
}
// ----------------------------------------

void CONTROL_NotifyCANbFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANbFault(Flag);
}
// ----------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState)
{
	// Set new state
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
}
// ----------------------------------------

void CONTROL_FillWPPartDefault()
{
	// Set states
	DataTable[REG_DEV_STATE] = DS_None;
	DataTable[REG_FAULT_REASON] = FAULT_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_FAULT_REASON_EXT] = 0;
	DataTable[REG_PULSES_COUNTER] = 0;
	//
	DataTable[REG_LOGIC_STATE] = 0;
	//
	DataTable[REG_SLAVE_DEVICE] = 0;
	DataTable[REG_SLAVE_FUNC] = 0;
	DataTable[REG_SLAVE_ERR] = 0;
	DataTable[REG_SLAVE_EXTDATA] = 0;
	//
	DataTable[REG_FINISHED] = OPRESULT_NONE;
	DataTable[REG_RES_QRR] = 0;
	DataTable[REG_RES_IRR] = 0;
	DataTable[REG_RES_TRR] = 0;
	DataTable[REG_RES_TQ] = 0;
	DataTable[REG_RES_IDC] = 0;
}
// ----------------------------------------

void CONTROL_SwitchToReady()
{
	CONTROL_SetDeviceState(DS_Ready);
}
// ----------------------------------------

void CONTROL_SwitchToFault(Int16U FaultReason, Int16U FaultReasonExt)
{
	CONTROL_Commutation(FALSE);

	CONTROL_SetDeviceState(DS_Fault);
	DataTable[REG_FAULT_REASON] = FaultReason;
	DataTable[REG_FAULT_REASON_EXT] = FaultReasonExt;
}
// ----------------------------------------

void CONTROL_StatusHLI()
{
	HLIError err = HLI_GetError();
	if(err.ErrorCode != ERR_NO_ERROR)
	{
		DataTable[REG_SLAVE_DEVICE] = err.Device;
		DataTable[REG_SLAVE_FUNC] = err.Func;
		DataTable[REG_SLAVE_ERR] = err.ErrorCode;
		DataTable[REG_SLAVE_EXTDATA] = err.ExtData;
	}
}
// ----------------------------------------

void CONTROL_ReinitRS232()
{
	if(!ReinitRS232 && CONTROL_TimeCounter > TIMEOUT_REINIT_RS232)
	{
		ZwSCIb_Init(SCIB_BR, SCIB_DB, SCIB_PARITY, SCIB_SB, FALSE);
		ZwSCIb_InitFIFO(16, 0);
		ZwSCIb_EnableInterrupts(FALSE, FALSE);
		
		ReinitRS232 = TRUE;
	}
}
// ----------------------------------------

void CONTROL_Start(Boolean SinglePulse)
{
	DEVPROFILE_ResetEPReadState();
	DEVPROFILE_ResetScopes(0, 0xFFFF);
	
	CONTROL_FillWPPartDefault();
	
	CONTROL_PulseToPulsePause = CONTROL_TimeCounter;
	LOGIC_CacheUpdateSettings(TRUE, SinglePulse);
	
	CONTROL_Commutation(TRUE);
	CONTROL_CommutationDelay = CONTROL_TimeCounter + DELAY_COMMUTATION;

	CONTROL_SetDeviceState(DS_InProcess);
	LOGIC_SetState(LS_WaitCommutation);
}
// ----------------------------------------

void CONTROL_SubProcessStateMachine()
{
	if(CONTROL_State == DS_PowerOn)
	{
		if(LOGIC_GetState() == LS_None)
			CONTROL_SetDeviceState(DS_Ready);
	}
	
	if(CONTROL_State == DS_InProcess)
	{
		if(LOGIC_GetState() == LS_WaitCommutation && CONTROL_TimeCounter > CONTROL_CommutationDelay)
		{
			LOGIC_StateRealTime = LSRT_WaitForConfig;
			LOGIC_ConfigurePrepare();
		}

		if(LOGIC_StateRealTime == LSRT_WaitForConfig)
		{
			if(LOGIC_GetState() == LS_None && CONTROL_TimeCounter > CONTROL_PulseToPulsePause)
			{
				if(MeasurementMode == MODE_DVDT_ONLY)
				{
					ZbGPIO_FCROVU_Sync(TRUE);
					DELAY_US(SYNC_WIDTH_CROVU);
					ZbGPIO_FCROVU_Sync(FALSE);

					DELAY_US(500);

					CONTROL_Commutation(FALSE);

					Int16U Register;
					if(HLI_CAN_Read16(DataTable[REG_CROVU_NODE_ID], COMM_REG_OP_RESULT, &Register))
					{
						DataTable[REG_FINISHED] = Register;
						CONTROL_SwitchToReady();
					}
					else
						CONTROL_SwitchToFault(FAULT_PROTOCOL, FAULTEX_READ_TIMEOUT);
				}
				else
				{
					LOGIC_GenerateSyncSequence();
					LOGIC_RealTimeCounter = 0;

					LOGIC_StateRealTime = LSRT_DirectPulseStart;
				}
			}
		}
		else if(LOGIC_GetState() == LS_None && LOGIC_StateRealTime == LSRT_None)
		{
			if(LOGIC_GetPulsesRemain() == 0)
			{
				LOGIC_ResultToDataTable();
				DataTable[REG_FINISHED] = LOGIC_GetOpResult();
				CONTROL_SwitchToReady();
				CONTROL_Commutation(FALSE);
			}
			else
			{
				CONTROL_PulseToPulsePause = CONTROL_TimeCounter
						+ (LOGIC_DCPulseFormed() ?
								(LOGIC_DUTTriggered() ? UNIT_PULSE_TO_PULSE_PAUSE : UNIT_PULSE_TO_PULSE_FAST) :
								UNIT_PULSE_TO_PULSE_LONG);
				
				LOGIC_StateRealTime = LSRT_WaitForConfig;
				LOGIC_ConfigurePrepare();
			}
			
		}
	}
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			{
				if(CONTROL_State == DS_None)
				{
					CONTROL_SetDeviceState(DS_PowerOn);
					LOGIC_PowerOnPrepare();
				}
				else if(CONTROL_State != DS_PowerOn && CONTROL_State != DS_Ready)
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DISABLE_POWER:
			{
				if(CONTROL_State == DS_Ready | CONTROL_State == DS_PowerOn)
				{
					CONTROL_SetDeviceState(DS_None);
					LOGIC_PowerOffPrepare();
				}
				else if(CONTROL_State != DS_None)
					*UserError = ERR_DEVICE_NOT_READY;
			}
			break;
			
		case ACT_START:
			{
				(CONTROL_State == DS_Ready) ? CONTROL_Start(FALSE) : (*UserError = ERR_DEVICE_NOT_READY);
			}
			break;
			
		case ACT_SINGLE_START:
			{
				(CONTROL_State == DS_Ready) ? CONTROL_Start(TRUE) : (*UserError = ERR_DEVICE_NOT_READY);
			}
			break;
			
		case ACT_STOP:
			{
				LOGIC_AbortMeasurement(WARNING_MANUAL_STOP);
				CONTROL_Commutation(FALSE);
				CONTROL_SwitchToReady();
			}
			break;

		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault || CONTROL_State == DS_None)
			{
				CONTROL_RequestDPC(NULL);
				LOGIC_ResetState();
				HLI_ResetError();
				CONTROL_FillWPPartDefault();
				
				if(CONTROL_State == DS_Fault)
				{
					LOGIC_CacheUpdateSettings(FALSE, FALSE);
					LOGIC_FaultResetPrepare();
					CONTROL_SetDeviceState(DS_None);
				}
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;
			
		case ACT_DIAG_READ_REG:
			{
				Int16U val;
				Boolean ret =
						(DataTable[REG_DIAG_NID] == 0) ?
								HLI_RS232_Read16(DataTable[REG_DIAG_IN_1], &val) :
								HLI_CAN_Read16(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1], &val);
				if(ret)
				{
					DataTable[REG_DIAG_OUT_1] = ERR_NO_ERROR;
					DataTable[REG_DIAG_OUT_2] = val;
				}
				else
				{
					HLIError err = HLI_GetError();
					DataTable[REG_DIAG_OUT_1] = err.ErrorCode;
				}
			}
			break;
			
		case ACT_DIAG_WRITE_REG:
			{
				Boolean ret =
						(DataTable[REG_DIAG_NID] == 0) ?
								HLI_RS232_Write16(DataTable[REG_DIAG_IN_1], DataTable[REG_DIAG_IN_2]) :
								HLI_CAN_Write16(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1],
										DataTable[REG_DIAG_IN_2]);
				if(ret)
					DataTable[REG_DIAG_OUT_1] = ERR_NO_ERROR;
				else
				{
					HLIError err = HLI_GetError();
					DataTable[REG_DIAG_OUT_1] = err.ErrorCode;
				}
			}
			break;
			
		case ACT_DIAG_CALL:
			{
				Boolean ret =
						(DataTable[REG_DIAG_NID] == 0) ?
								HLI_RS232_CallAction(DataTable[REG_DIAG_IN_1]) :
								HLI_CAN_CallAction(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1]);
				if(ret)
					DataTable[REG_DIAG_OUT_1] = ERR_NO_ERROR;
				else
				{
					HLIError err = HLI_GetError();
					DataTable[REG_DIAG_OUT_1] = err.ErrorCode;
				}
			}
			break;
			
		case ACT_DIAG_READ_EP:
			{
				DEVPROFILE_ResetEPReadState();
				DEVPROFILE_ResetScopes(0, BIT10);
				
				Boolean ret =
						(DataTable[REG_DIAG_NID] == 0) ?
								HLI_RS232_ReadArray16(DataTable[REG_DIAG_IN_1], CONTROL_Values_Slave, VALUES_x_SIZE,
										(pInt16U)&CONTROL_Values_Slave_Counter) :
								HLI_CAN_ReadArray16(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1],
										CONTROL_Values_Slave, VALUES_x_SIZE, (pInt16U)&CONTROL_Values_Slave_Counter);
				if(ret)
				{
					DataTable[REG_DIAG_OUT_1] = ERR_NO_ERROR;
					DataTable[REG_DIAG_OUT_2] = CONTROL_Values_Slave_Counter;
				}
				else
				{
					HLIError err = HLI_GetError();
					DataTable[REG_DIAG_OUT_1] = err.ErrorCode;
				}
			}
			break;
			
		case ACT_DIAG_FILL_RESULTS:
			{
				Int16U i;
				MeasurementResult Result;
				
				DEVPROFILE_ResetEPReadState();
				DEVPROFILE_ResetScopes(0, 0xFFFF);
				
				for(i = 0; i < UNIT_MAX_NUM_OF_PULSES; ++i)
				{
					Result.DeviceTriggered = i % 2;
					Result.OSVApplyTime = i + 10;
					Result.Irr = i + 20;
					Result.Trr = i + 30;
					Result.Qrr = i + 40;
					Result.Idc = i + 50;
					Result.ZeroI = i + 60;
					Result.ZeroV = i + 70;
					
					LOGIC_LogData(Result);
				}
			}
			break;
			
		case ACT_DIAG_TURN_ON_PC:
			{
				ZbGPIO_PC_TurnOn();
			}
			break;
			
		case ACT_DIAG_PULSE_DUT:
			{
				ZbGPIO_DUT_Control(TRUE);
				DELAY_US(1000);
				ZbGPIO_DUT_Control(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_SW_DUT:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_DUT_Switch(TRUE);
				else
					ZbGPIO_DUT_Switch(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_DC_SYNC:
			{
				ZbGPIO_DCU_Sync(TRUE);
				DELAY_US(1000);
				ZbGPIO_DCU_Sync(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_RC_SYNC:
			{
				ZbGPIO_RCU_Sync(TRUE);
				DELAY_US(1000);
				ZbGPIO_RCU_Sync(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_CSU_PS:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_CSU_PWRCtrl(TRUE);
				else
					ZbGPIO_CSU_PWRCtrl(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_CSU_DISCH:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_CSU_Disch(TRUE);
				else
					ZbGPIO_CSU_Disch(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_CSU_FAN:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_CSU_FAN(TRUE);
				else
					ZbGPIO_CSU_FAN(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_CSU_SYNC:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_CSU_Sync(TRUE);
				else
					ZbGPIO_CSU_Sync(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_SCOPE:
			{
				ZbGPIO_SCOPE_Sync(TRUE);
				DELAY_US(1000);
				ZbGPIO_SCOPE_Sync(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_FCROVU:
			{
				ZbGPIO_FCROVU_Sync(TRUE);
				DELAY_US(200);
				ZbGPIO_FCROVU_Sync(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_SB:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_SensingBoardEnable(TRUE);
				else
					ZbGPIO_SensingBoardEnable(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_QCUHC:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_QCUHCEnable(TRUE);
				else
					ZbGPIO_QCUHCEnable(FALSE);
			}
			break;
			
		case ACT_DIAG_PULSE_GATE_RELAY:
			{
				if(DataTable[REG_DBG])
					ZbGPIO_DUT_ControlEnable(TRUE);
				else
					ZbGPIO_DUT_ControlEnable(FALSE);
			}
			break;
			
		case ACT_DIAG_QRR_PULSE:
			{
				int i;
				
				ZbGPIO_DCU_Sync(TRUE);
				DELAY_US(3000);
				
				if(DataTable[REG_RCU_SYNC_DELAY] - DataTable[REG_DCU_SYNC_DELAY])
				{
					if(DataTable[REG_DCU_SYNC_DELAY])
					{
						ZbGPIO_RCU_Sync(TRUE);
						for(i = 0; i < DataTable[REG_DCU_SYNC_DELAY]; i++);
						ZbGPIO_DCU_Sync(FALSE);
					}
					else
					{
						ZbGPIO_DCU_Sync(FALSE);
						for(i = 0; i < DataTable[REG_RCU_SYNC_DELAY]; i++);
						ZbGPIO_RCU_Sync(TRUE);
					}
				}
				else
				{
					ZbGPIO_DCU_Sync(FALSE);
					ZbGPIO_RCU_Sync(TRUE);
				}
				
				DELAY_US(1000);
				ZbGPIO_RCU_Sync(FALSE);
			}
			break;

		case ACT_COMMUTATION_FORCED_ON:
			{
				CONTROL_Commutation(TRUE);
				CommutationForcedOn = TRUE;
			}
			break;
			
		case ACT_COMMUTATION_FORCED_OFF:
			{
				CommutationForcedOn = FALSE;
				CONTROL_Commutation(FALSE);
			}
			break;

		default:
			return FALSE;
	}
	
	return TRUE;
}
// ----------------------------------------

void CONTROL_Commutation(Boolean State)
{
	if(!CommutationForcedOn)
	{
		ZbGPIO_QCUHCEnable(State);
		ZbGPIO_SensingBoardEnable(State);
		ZbGPIO_DUT_ControlEnable(State);
	}
}
// ----------------------------------------

void CONTROL_SafetyHandler()
{
	if((CONTROL_State == DS_InProcess) && ZbGPIO_SafetyCheck())
		LOGIC_SafetyProblem();
	
	// Safety system enable
	ZbGPIO_SafetyEnable(DataTable[REG_SAFETY_EN]);
}
// ----------------------------------------

void CONTROL_PressureHandler()
{
	static Int16U PressureFaultCounter = 0;

	if(((CONTROL_State == DS_InProcess) || (CONTROL_State == DS_Ready)) && !ZbGPIO_PressureCheck())
	{
		PressureFaultCounter++;

		if(PressureFaultCounter > PRESSURE_FAULT_COUNTER_MAX)
			CONTROL_SwitchToFault(FAULT_PRESSURE, 0);
	}
	else
		PressureFaultCounter = 0;

	DataTable[REG_PRESSURE] = ZbGPIO_PressureCheck();
}
// ----------------------------------------
