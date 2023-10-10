// -----------------------------------------
// High-level master interface
// ----------------------------------------

#ifndef __HIGHLEVELINTERFACE_H
#define __HIGHLEVELINTERFACE_H

// Include
#include "stdinc.h"

// Types
//
typedef struct __HLIError
{
	Int16U ErrorCode;
	Int16U Device;
	Int16U Func;
	Int16U ExtData;
} HLIError, *pHLIError;

// Functions
//
Boolean HLI_RS232_Read16(Int16U Register, pInt16U Value);
Boolean HLI_RS232_Write16(Int16U Register, Int16U Value);
Boolean HLI_RS232_CallAction(Int16U ActionID);
Boolean HLI_RS232_ReadArray16(Int16U Endpoint, pInt16U Data, Int16U DataSize, pInt16U DataRead);
Boolean HLI_RS232_ReadArray16CB(Int16U Endpoint, pInt16U Data, Int16U DataSize, pInt16U DataRead);
//
Boolean HLI_CAN_Read16(Int16U Node, Int16U Register, pInt16U Value);
Boolean HLI_CAN_Write16(Int16U Node, Int16U Register, Int16U Value);
Boolean HLI_CAN_CallAction(Int16U Node, Int16U ActionID);
Boolean HLI_CAN_ReadArray16(Int16U Node, Int16U Endpoint, pInt16U Data, Int16U DataSize, pInt16U DataRead);
//
HLIError HLI_GetError();
void HLI_ResetError();

#endif // __HIGHLEVELINTERFACE_H
