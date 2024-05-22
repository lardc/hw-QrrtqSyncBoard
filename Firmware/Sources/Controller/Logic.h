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
	//
	LS_PON_FCROVU		= 2,
	LS_PON_QPU			= 3,
	LS_PON_SCOPE		= 4,
	LS_PON_WaitStates	= 5,
	//
	LS_CFG_FCROVU		= 6,
	LS_CFG_QPU			= 7,
	LS_CFG_SCOPE		= 8,
	LS_CFG_WaitStates	= 9,
	//
	LS_POFF_FCROVU		= 10,
	LS_POFF_QPU			= 11,
	LS_POFF_SCOPE		= 12,
	//
	LS_READ_FCROVU		= 13,
	LS_READ_QPU			= 14,
	LS_READ_SCOPE		= 15,
	//
	LS_CLR_FCROVU		= 16,
	LS_CLR_QPU			= 17,
	LS_CLR_SCOPE		= 18

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
typedef struct __MeasurementResult
{
	Boolean DeviceTriggered;	// If device opened by on-state voltage
	Int16U OSVApplyTime;		// Time of applying of on-state voltage (in us x10)
	Int16U Irr;					// Reverse recovery current amplitude (in A)
	Int16U Trr;					// Reverse recovery time (in us x10)
	Int16U Qrr;					// Reverse recovery charge (in uQ)
	Int16U Idc;					// Actual direct current (in A)
	Int16U ZeroI;				// Time of current zero-crossing (in us x10)
	Int16U ZeroV;				// Time of voltage zero-crossing (in us x10)
	Int16U dIdt;				// Actual dIdt value (in A/us x10)
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

// Variables
//
extern volatile DeviceSubState LOGIC_StateRealTime;
extern volatile Int32U LOGIC_RealTimeCounter;

#endif // __LOGIC_H
