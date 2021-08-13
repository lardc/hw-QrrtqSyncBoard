// -----------------------------------------
// High-level master interface
// ----------------------------------------

// Header
#include "HighLevelInterface.h"
//
// Includes
#include "Global.h"
#include "DeviceProfile.h"
#include "SCCIMaster.h"
#include "BCCIMaster.h"

// Variables
//
static HLIError HLI_Error = {ERR_NO_ERROR, 0, 0};

// Functions
//
void HLI_LoadError(Int16U Code, Int16U NodeID, Int16U Func, Int16U ExtData)
{
	HLI_Error.ErrorCode = Code;
	HLI_Error.Device = NodeID;
	HLI_Error.Func = Func;
	HLI_Error.ExtData = ExtData;
}
// ----------------------------------------

void HLI_ResetError()
{
	HLI_Error.ErrorCode = ERR_NO_ERROR;
	HLI_Error.Device = 0;
	HLI_Error.Func = 0;
	HLI_Error.ExtData = 0;
}
// ----------------------------------------

void HLI_RS232_ClearRX()
{
	while (DEVICE_RS232_Master_Interface.IOConfig->IO_GetBytesToReceive())
		DEVICE_RS232_Master_Interface.IOConfig->IO_ReceiveByte();
}
// ----------------------------------------

Boolean HLI_RS232_Read16(Int16U Register, pInt16U Value)
{
	Int16U err;

	HLI_RS232_ClearRX();
	err = SCCIM_Read16(&DEVICE_RS232_Master_Interface, NODEID_SCCI_SCOPE, Register, Value);

	if (err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, NODEID_SCCI_SCOPE, FUNCTION_READ, Register);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_RS232_Write16(Int16U Register, Int16U Value)
{
	Int16U err;

	HLI_RS232_ClearRX();
	err = SCCIM_Write16(&DEVICE_RS232_Master_Interface, NODEID_SCCI_SCOPE, Register, Value);

	if (err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, NODEID_SCCI_SCOPE, FUNCTION_WRITE, Register);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_RS232_CallAction(Int16U ActionID)
{
	Int16U err;

	HLI_RS232_ClearRX();
	err = SCCIM_Call(&DEVICE_RS232_Master_Interface, NODEID_SCCI_SCOPE, ActionID);

	if (err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, NODEID_SCCI_SCOPE, FUNCTION_CALL, ActionID);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_RS232_ReadArray16(Int16U Endpoint, pInt16U Data, Int16U DataSize, pInt16U DataRead)
{
	Int16U err;

	HLI_RS232_ClearRX();
	err = SCCIM_ReadArray16Callback(&DEVICE_RS232_Master_Interface, NODEID_SCCI_SCOPE, Endpoint, DataSize, Data,
			DataRead, &DEVPROFILE_ProcessRequestsBCCI);

	if(err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, NODEID_SCCI_SCOPE, FUNCTION_READ_BLOCK, Endpoint);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_CAN_Read16(Int16U Node, Int16U Register, pInt16U Value)
{
	Int16U err;
	err = BCCIM_Read16(&DEVICE_CAN_Master_Interface, Node, Register, Value);

	if (err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, Node, FUNCTION_READ, Register);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_CAN_Write16(Int16U Node, Int16U Register, Int16U Value)
{
	Int16U err;
	err = BCCIM_Write16(&DEVICE_CAN_Master_Interface, Node, Register, Value);

	if (err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, Node, FUNCTION_WRITE, Register);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_CAN_CallAction(Int16U Node, Int16U ActionID)
{
	Int16U err;
	err = BCCIM_Call(&DEVICE_CAN_Master_Interface, Node, ActionID);

	if (err == ERR_NO_ERROR)
		return TRUE;
	else
	{
		HLI_LoadError(err, Node, FUNCTION_CALL, ActionID);
		return FALSE;
	}
}
// ----------------------------------------

Boolean HLI_CAN_ReadArray16(Int16U Node, Int16U Endpoint, pInt16U Data, Int16U DataSize, pInt16U DataRead)
{
	Int16U err;
	err = BCCIM_ReadBlock16(&DEVICE_CAN_Master_Interface, Node, Endpoint);

	if (err == ERR_NO_ERROR)
	{
		BCCIM_ReadBlock16Load(Data, DataSize, DataRead);
		return TRUE;
	}
	else
	{
		*DataRead = 0;
		HLI_LoadError(err, Node, FUNCTION_CALL, Endpoint);
		return FALSE;
	}
}
// ----------------------------------------

HLIError HLI_GetError()
{
	return HLI_Error;
}
// ----------------------------------------

// No more.
