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

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile Int64U CONTROL_TimeCounter = 0, CONTROL_PulseToPulsePause;
static volatile Boolean CycleActive = FALSE, ReinitRS232 = FALSE;
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

// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	// Variables for endpoint configuration
	Int16U EPIndexes[EP_COUNT] = { EP_Current, EP_Voltage,
			EP_DIAG1_DevTrig, EP_DIAG2_OSVTime, EP_DIAG3_Irr, EP_DIAG4_Trr,
			EP_DIAG5_Qrr, EP_DIAG6_Idc, EP_DIAG7_ZeroI, EP_DIAG8_ZeroV, EP_DIAG9_dIdt, EP_SlaveData };

	Int16U EPSized[EP_COUNT] = { VALUES_x_SIZE, VALUES_x_SIZE,
			UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES,
			UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, UNIT_MAX_NUM_OF_PULSES, VALUES_x_SIZE };

	pInt16U EPCounters[EP_COUNT] = { (pInt16U)&CONTROL_Values_1_Counter, (pInt16U)&CONTROL_Values_2_Counter,
			(pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter,
			(pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_ValDiag_Counter,
			(pInt16U)&CONTROL_ValDiag_Counter, (pInt16U)&CONTROL_Values_Slave_Counter };

	pInt16U EPDatas[EP_COUNT] = { CONTROL_Values_1, CONTROL_Values_2,
			CONTROL_ValDiag1, CONTROL_ValDiag2, CONTROL_ValDiag3, CONTROL_ValDiag4,
			CONTROL_ValDiag5, CONTROL_ValDiag6, CONTROL_ValDiag7, CONTROL_ValDiag8, CONTROL_ValDiag9, CONTROL_Values_Slave };

	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = { &ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM };

	// Init data table
	DT_Init(EPROMService, BadClockDetected);
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

	// Update high-level logic state
	DataTable[REG_LOGIC_STATE] = LOGIC_GetState();

	// Process deferred procedures
	FUNC_AsyncDelegate DPCDelegateCopy = DPCDelegate;
	if (DPCDelegateCopy)
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
	DataTable[REG_DC_READY_RETRIES] = 0;

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
	CONTROL_SetDeviceState(DS_Fault);
	DataTable[REG_FAULT_REASON] = FaultReason;
	DataTable[REG_FAULT_REASON_EXT] = FaultReasonExt;
}
// ----------------------------------------

void CONTROL_StatusHLI()
{
	HLIError err = HLI_GetError();
	if (err.ErrorCode != ERR_NO_ERROR)
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
	if (!ReinitRS232 && CONTROL_TimeCounter > TIMEOUT_REINIT_RS232)
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

	CONTROL_SetDeviceState(DS_InProcess);
	LOGIC_StateRealTime = LSRT_WaitForConfig;

	CONTROL_PulseToPulsePause = CONTROL_TimeCounter;
	LOGIC_CacheUpdateSettings(TRUE, SinglePulse);
	LOGIC_ConfigurePrepare();
}
// ----------------------------------------

void CONTROL_SubProcessStateMachine()
{
	if (CONTROL_State == DS_PowerOn)
	{
		if (LOGIC_GetState() == LS_None)
			CONTROL_SetDeviceState(DS_Ready);
	}

	if (CONTROL_State == DS_InProcess)
	{
		if (LOGIC_StateRealTime == LSRT_WaitForConfig)
		{
			if (LOGIC_GetState() == LS_None && CONTROL_TimeCounter > CONTROL_PulseToPulsePause)
			{
				LOGIC_RealTimeCounter = 0;

				ZbGPIO_SwitchHVIGBT(TRUE);
				ZbGPIO_SwitchDUT(TRUE);
				ZbGPIO_SwitchDirectCurrent(TRUE);

				LOGIC_StateRealTime = LSRT_DirectPulseStart;
			}
		}
		else if (LOGIC_GetState() == LS_None && LOGIC_StateRealTime == LSRT_None)
		{
			if (LOGIC_GetPulsesRemain() == 0)
			{
				LOGIC_ResultToDataTable();
				DataTable[REG_FINISHED] = LOGIC_GetOpResult();
				CONTROL_SwitchToReady();
			}
			else
			{
				CONTROL_PulseToPulsePause = CONTROL_TimeCounter +
					(LOGIC_DCPulseFormed() ? (LOGIC_DUTTriggered() ? UNIT_PULSE_TO_PULSE_PAUSE : UNIT_PULSE_TO_PULSE_FAST) : UNIT_PULSE_TO_PULSE_LONG);

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
				if (CONTROL_State == DS_None)
				{
					CONTROL_SetDeviceState(DS_PowerOn);
					LOGIC_PowerOnPrepare();
				}
				else if (CONTROL_State != DS_PowerOn && CONTROL_State != DS_Ready)
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DISABLE_POWER:
			{
				if (CONTROL_State == DS_Ready)
				{
					CONTROL_SetDeviceState(DS_None);
					LOGIC_PowerOffPrepare();
				}
				else if (CONTROL_State != DS_None)
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
				CONTROL_SwitchToReady();
			}
			break;

		case ACT_CLR_FAULT:
			if (CONTROL_State == DS_Fault || CONTROL_State == DS_None)
			{
				CONTROL_RequestDPC(NULL);
				LOGIC_ResetState();
				HLI_ResetError();
				CONTROL_FillWPPartDefault();

				if (CONTROL_State == DS_Fault)
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
				Boolean ret = (DataTable[REG_DIAG_NID] == 0) ? 	HLI_RS232_Read16(DataTable[REG_DIAG_IN_1], &val) :
																HLI_CAN_Read16(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1], &val);
				if (ret)
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
				Boolean ret = (DataTable[REG_DIAG_NID] == 0) ? 	HLI_RS232_Write16(DataTable[REG_DIAG_IN_1], DataTable[REG_DIAG_IN_2]) :
																HLI_CAN_Write16(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1], DataTable[REG_DIAG_IN_2]);
				if (ret)
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
				Boolean ret = (DataTable[REG_DIAG_NID] == 0) ? 	HLI_RS232_CallAction(DataTable[REG_DIAG_IN_1]) :
																HLI_CAN_CallAction(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1]);
				if (ret)
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

				Boolean ret = (DataTable[REG_DIAG_NID] == 0) ? 	HLI_RS232_ReadArray16(DataTable[REG_DIAG_IN_1], CONTROL_Values_Slave, VALUES_x_SIZE, (pInt16U)&CONTROL_Values_Slave_Counter) :
																HLI_CAN_ReadArray16(DataTable[REG_DIAG_NID], DataTable[REG_DIAG_IN_1], CONTROL_Values_Slave, VALUES_x_SIZE, (pInt16U)&CONTROL_Values_Slave_Counter);
				if (ret)
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

				for (i = 0; i < UNIT_MAX_NUM_OF_PULSES; ++i)
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
				ZbGPIO_SwitchLED2(TRUE);
				DELAY_US(500000);
				ZbGPIO_SwitchLED2(FALSE);
			}
			break;

		case ACT_DIAG_PULSE_DUT:
			{
				ZbGPIO_SwitchDUT(TRUE);
				DELAY_US(1000);
				ZbGPIO_SwitchDUT(FALSE);
			}
			break;

		case ACT_DIAG_PULSE_DC:
			{
				ZbGPIO_SwitchDirectCurrent(TRUE);
				DELAY_US(1000);
				ZbGPIO_SwitchDirectCurrent(FALSE);
			}
			break;

		case ACT_DIAG_PULSE_RC:
			{
				ZbGPIO_SwitchReverseCurrent(TRUE);
				DELAY_US(1000);
				ZbGPIO_SwitchReverseCurrent(FALSE);
			}
			break;

		case ACT_DIAG_PULSE_IGBT:
			{
				ZbGPIO_SwitchHVIGBT(TRUE);
				DELAY_US(1000);
				ZbGPIO_SwitchHVIGBT(FALSE);
			}
			break;

		case ACT_DIAG_PULSE_SCOPE:
			{
				ZbGPIO_SyncSCOPE(TRUE);
				DELAY_US(1000);
				ZbGPIO_SyncSCOPE(FALSE);
			}
			break;

		case ACT_DIAG_PULSE_FCROVU:
			{
				ZbGPIO_SyncFCROVU(TRUE);
				DELAY_US(200);
				ZbGPIO_SyncFCROVU(FALSE);
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

// No more.
