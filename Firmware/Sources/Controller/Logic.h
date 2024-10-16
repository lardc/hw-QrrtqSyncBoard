// -----------------------------------------
// Main logic library
// ----------------------------------------

#ifndef __LOGIC_H
#define __LOGIC_H

// Include
//
#include "stdinc.h"

// Definitions
//
typedef enum __LogicState
{
	LS_None				= 0,
	LS_Error			= 1,
	LS_WaitCommutation,
	//
	LS_CLR_CROVU		= 100,
	LS_CLR_FCROVU,
	LS_CLR_DCU1,
	LS_CLR_DCU2,
	LS_CLR_DCU3,
	LS_CLR_RCU1,
	LS_CLR_RCU2,
	LS_CLR_RCU3,
	LS_CLR_SCOPE,
	//
	LS_PON_CROVU		= 200,
	LS_PON_FCROVU,
	LS_PON_DCU1,
	LS_PON_DCU2,
	LS_PON_DCU3,
	LS_PON_RCU1,
	LS_PON_RCU2,
	LS_PON_RCU3,
	LS_PON_CSU,
	LS_PON_SCOPE,
	LS_PON_WaitStates,
	//
	LS_CFG_WaitReady	= 300,
	LS_CFG_CROVU,
	LS_CFG_FCROVU,
	LS_CFG_DCU1,
	LS_CFG_DCU2,
	LS_CFG_DCU3,
	LS_CFG_RCU1,
	LS_CFG_RCU2,
	LS_CFG_RCU3,
	LS_CFG_SCOPE,
	LS_CFG_WaitStates,
	//
	LS_POFF_CROVU		= 400,
	LS_POFF_FCROVU,
	LS_POFF_DCU1,
	LS_POFF_DCU2,
	LS_POFF_DCU3,
	LS_POFF_RCU1,
	LS_POFF_RCU2,
	LS_POFF_RCU3,
	LS_POFF_CSU,
	LS_POFF_SCOPE,
	//
	LS_READ_CROVU		= 500,
	LS_READ_FCROVU,
	LS_READ_DCU,
	LS_READ_RCU,
	LS_READ_CSU,
	LS_READ_SCOPE
} LogicState;
//
typedef enum __DeviceSubState
{
	LSRT_None					= 0,
	LSRT_WaitForConfig			= 1,
	LSRT_DirectPulseStart		= 2,
	LSRT_DirectPulseReady		= 3,
	LSRT_ReversePulseStart		= 4,
	LSRT_ReadDataPause			= 5

} DeviceSubState;
//
typedef struct __DeviceStateEntity
{
	Int16U State;
	Boolean Emulate;
} DeviceStateEntity;
//
typedef struct __ExternalDeviceState
{
	DeviceStateEntity CROVU;
	DeviceStateEntity FCROVU;
	DeviceStateEntity DCU1;
	DeviceStateEntity DCU2;
	DeviceStateEntity DCU3;
	DeviceStateEntity RCU1;
	DeviceStateEntity RCU2;
	DeviceStateEntity RCU3;
	DeviceStateEntity CSU;
	DeviceStateEntity SCOPE;
} ExternalDeviceState;
//
typedef struct __MeasurementResult
{
	Boolean DeviceTriggered;	// If device opened by on-state voltage
	Int16U OSVApplyTime;		// Time of applying of on-state voltage (in us x10)
	Int16U Irr;					// Reverse recovery current amplitude (in A)
	Int16U Trr;					// Reverse recovery time (in us x10)
	Int32U Qrr;					// Reverse recovery charge (in uQ)
	Int16U Idc;					// Actual direct current (in A)
	Int16U Vd;					// Actual direct voltage (in V)
	Int16U ZeroI;				// Time of current zero-crossing (in us x10)
	Int16U ZeroV;				// Time of voltage zero-crossing (in us x10)
	Int16U dIdt;				// Actual dIdt value (in A/us x10)
	Int16U EPTimeFract;			// Time fraction for EP data step (in ns)
	Int16U EPTimeFractCnt;		// Number of time fractions in data step
} MeasurementResult;

// Functions
//
// Handle real-time process
void LOGIC_RealTime();
// Get number of pulses remain
Int16U LOGIC_GetPulsesRemain();
// Get logic operation result
Int16U LOGIC_GetOpResult();
// Check if DC pulse was formed
Boolean LOGIC_DCPulseFormed();
// Check if DUT was triggered
Boolean LOGIC_DUTTriggered();
// Read current state
LogicState LOGIC_GetState();
// Set current state
void LOGIC_SetState(LogicState State);
// Halt process
void LOGIC_Halt();
void LOGIC_ResetState();
// Configure cache update settings
void LOGIC_CacheUpdateSettings(Boolean UpdateMainSettings, Boolean SinglePulseMode);
// PowerOn functions
void LOGIC_PowerOnPrepare();
void LOGIC_PowerOnSequence();
// Configure functions
void LOGIC_ConfigurePrepare();
void LOGIC_ConfigureSequence();
// PowerOff functions
void LOGIC_PowerOffPrepare();
void LOGIC_PowerOffSequence();
// ReadData functions
void LOGIC_ReadDataPrepare();
void LOGIC_ReadDataSequence();
// Clear fault functions
void LOGIC_FaultResetPrepare();
void LOGIC_FaultResetSequence();
// Put result to DataTable
void LOGIC_ResultToDataTable();
// Log results to endpoint
void LOGIC_LogData(MeasurementResult Result);
// Unexpected abortion of measurement
void LOGIC_AbortMeasurement(Int16U WarningCode);
// Control CSU
void LOGIC_FanAndVoltageControlCSU();
// CSU voltage measuring
void LOGIC_VoltageMeasuringCSU(Int16U * const restrict pResults);
// Safety problem
void LOGIC_SafetyProblem();
// Generate sync pulses
void LOGIC_GenerateSyncSequence();
// Логика включения синхронизации CROVU/FCROVU
Int16U LOGIC_FCROVUOnSync(Int16U Delay);

// Variables
//
extern volatile DeviceSubState LOGIC_StateRealTime;
extern volatile Int32U LOGIC_RealTimeCounter;
extern volatile Int16U MeasurementMode;
extern volatile Int32U FCROVUTrigOffset;
#endif // __LOGIC_H
