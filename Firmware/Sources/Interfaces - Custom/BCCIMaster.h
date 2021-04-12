// -----------------------------------------
// BCCI master communication interface
// ----------------------------------------

#ifndef __BCCI_MASTER_H
#define __BCCI_MASTER_H

// Include
#include "stdinc.h"
//
#include "BCCITypes.h"


// Types
//
// BCCIM instance state
typedef struct __BCCIM_Interface
{
	pBCCI_IOConfig IOConfig;
	Int32U TimeoutValueTicks;
	volatile Int64U *pTimerCounter;
} BCCIM_Interface, *pBCCIM_Interface;
//
// BCCIM data exchange
typedef struct __BCCIM_DataExchange
{
	Int16U SavedID;
	Int16U Address;
	Int16U Data;
	Int16U Action;
	Int16U Code;
	Int16U Details;
	Boolean Finished;
	Boolean Error;
} BCCIM_DataExchange, *pBCCIM_DataExchange;


// Variables
//
extern volatile BCCIM_DataExchange BCCIM_RecievedData;


// Functions
//
// Init interface instance
void BCCIM_Init(pBCCIM_Interface Interface, pBCCI_IOConfig IOConfig,  Int32U MessageTimeoutTicks, volatile Int64U *pTimer);
// Process packets
void BCCIM_Process(pBCCIM_Interface Interface);
//
// Read 16bit value
Int16U BCCIM_Read16(pBCCIM_Interface Interface, Int16U Node, Int16U Address, pInt16U Value);
// Write 16bit value
Int16U BCCIM_Write16(pBCCIM_Interface Interface, Int16U Node, Int16U Address, Int16U Value);
// Call function
Int16U BCCIM_Call(pBCCIM_Interface Interface, Int16U Node, Int16U Action);
// Read block 16bit value
Int16U BCCIM_ReadBlock16(pBCCIM_Interface Interface, Int16U Node, Int16U Endpoint);
// Copy received data to output
void BCCIM_ReadBlock16Load(pInt16U DataArray, Int16U DataSize, pInt16U DataRead);

#endif // __BCCI_MASTER_H
