// -----------------------------------------
// BCCI communication interface
// ----------------------------------------

// Header
#include "BCCIMaster.h"
//
// Includes
#include "CRC16.h"
#include "SysConfig.h"
#include "DeviceProfile.h"
#include "Controller.h"


// Macro
//
#define CAN_ID_W_16					10
#define CAN_ID_R_16					20
#define CAN_ID_RB_16				40
#define CAN_ID_CALL					50
#define CAN_ID_ERR					61
//
#define MBOX_W_16					1
#define MBOX_W_16_A					2
#define MBOX_R_16					7
#define MBOX_R_16_A					8
#define MBOX_C						13
#define MBOX_C_A					14
#define MBOX_ERR_A					15
#define MBOX_RB_16					16
#define MBOX_RB_16_A				17
//
#define READ_BLOCK_16_BUFFER_SIZE	2000


// Forward functions
//
static void BCCIM_HandleRead16(pBCCIM_Interface Interface);
static void BCCIM_HandleWrite16(pBCCIM_Interface Interface);
static void BCCIM_HandleCall(pBCCIM_Interface Interface);
static void BCCIM_HandleReadBlock16(pBCCIM_Interface Interface);
static void BCCIM_HandleError(pBCCIM_Interface Interface);
//
void BCCIM_ReadBlock16Subfunction(pBCCIM_Interface Interface, Int16U Node, Int16U Endpoint, Boolean Start);
//
static void BCCIM_SendFrame(pBCCIM_Interface Interface, Int16U Mailbox, pCANMessage Message, Int32U Node, Int16U Command);
static void BCCIM_PrepareDataExchange(Int16U Code);
//
static Int16U ReadBlock16Buffer[READ_BLOCK_16_BUFFER_SIZE], ReadBlock16BufferCounter;
static Int16U SavedEndpointRB16;
//
volatile BCCIM_DataExchange BCCIM_ReceivedData;


// Functions
//
void BCCIM_Init(pBCCIM_Interface Interface, pBCCI_IOConfig IOConfig,  Int32U MessageTimeoutTicks, volatile Int64U *pTimer)
{
	// Save parameters
	Interface->IOConfig = IOConfig;
	Interface->TimeoutValueTicks = MessageTimeoutTicks;
	Interface->pTimerCounter = pTimer;

	// Setup messages
	Interface->IOConfig->IO_ConfigMailbox(MBOX_R_16, CAN_ID_R_16, FALSE, 2, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_R_16_A, CAN_ID_R_16 + 1, TRUE, 4, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, CAN_ACCEPTANCE_MASK);

	Interface->IOConfig->IO_ConfigMailbox(MBOX_W_16, CAN_ID_W_16, FALSE, 4, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_W_16_A, CAN_ID_W_16 + 1, TRUE, 2, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, CAN_ACCEPTANCE_MASK);

	Interface->IOConfig->IO_ConfigMailbox(MBOX_C, CAN_ID_CALL, FALSE, 2, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_C_A, CAN_ID_CALL + 1, TRUE, 2, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, CAN_ACCEPTANCE_MASK);

	Interface->IOConfig->IO_ConfigMailbox(MBOX_RB_16, CAN_ID_RB_16, FALSE, 2, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_RB_16_A, CAN_ID_RB_16 + 1, TRUE, 8, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, CAN_ACCEPTANCE_MASK);

	Interface->IOConfig->IO_ConfigMailbox(MBOX_ERR_A, CAN_ID_ERR, TRUE, 4, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, CAN_ACCEPTANCE_MASK);
}
// ----------------------------------------

void BCCIM_Process(pBCCIM_Interface Interface)
{
	if(Interface->IOConfig->IO_IsMessageReceived(MBOX_R_16_A, NULL)) { BCCIM_HandleRead16(Interface); return; }
	if(Interface->IOConfig->IO_IsMessageReceived(MBOX_W_16_A, NULL)) { BCCIM_HandleWrite16(Interface); return; }
	if(Interface->IOConfig->IO_IsMessageReceived(MBOX_C_A, NULL)) { BCCIM_HandleCall(Interface); return; }
	if(Interface->IOConfig->IO_IsMessageReceived(MBOX_RB_16_A, NULL)) { BCCIM_HandleReadBlock16(Interface); return; }
	if(Interface->IOConfig->IO_IsMessageReceived(MBOX_ERR_A, NULL)) { BCCIM_HandleError(Interface); return; }
}
// ----------------------------------------

Int16U BCCIM_Read16(pBCCIM_Interface Interface, Int16U Node, Int16U Address, pInt16U Value)
{
	Int64U timeout;
	CANMessage message;

	BCCIM_PrepareDataExchange(CAN_ID_R_16);
	message.HIGH.WORD.WORD_0 = Address;
	BCCIM_SendFrame(Interface, MBOX_R_16, &message, Node, CAN_ID_R_16);

	timeout = Interface->TimeoutValueTicks + *(Interface->pTimerCounter);
	while(*(Interface->pTimerCounter) < timeout)
	{
		if (BCCIM_ReceivedData.Finished)
		{
			if (BCCIM_ReceivedData.Error)
				return BCCIM_ReceivedData.Code;
			else
			{
				*Value = BCCIM_ReceivedData.Data;
				return ERR_NO_ERROR;
			}
		}
	}

	return ERR_TIMEOUT;
}
// ----------------------------------------

Int16U BCCIM_Write16(pBCCIM_Interface Interface, Int16U Node, Int16U Address, Int16U Value)
{
	Int64U timeout;
	CANMessage message;

	BCCIM_PrepareDataExchange(CAN_ID_W_16);
	message.HIGH.WORD.WORD_0 = Address;
	message.HIGH.WORD.WORD_1 = Value;
	BCCIM_SendFrame(Interface, MBOX_W_16, &message, Node, CAN_ID_W_16);

	timeout = Interface->TimeoutValueTicks + *(Interface->pTimerCounter);
	while(*(Interface->pTimerCounter) < timeout)
	{
		if (BCCIM_ReceivedData.Finished)
		{
			return (BCCIM_ReceivedData.Error) ? BCCIM_ReceivedData.Code : ERR_NO_ERROR;
		}
	}

	return ERR_TIMEOUT;
}
// ----------------------------------------

Int16U BCCIM_Call(pBCCIM_Interface Interface, Int16U Node, Int16U Action)
{
	Int64U timeout;
	CANMessage message;

	BCCIM_PrepareDataExchange(CAN_ID_CALL);
	message.HIGH.WORD.WORD_0 = Action;
	BCCIM_SendFrame(Interface, MBOX_C, &message, Node, CAN_ID_CALL);

	timeout = Interface->TimeoutValueTicks + *(Interface->pTimerCounter);
	while(*(Interface->pTimerCounter) < timeout)
	{
		if (BCCIM_ReceivedData.Finished)
		{
			return (BCCIM_ReceivedData.Error) ? BCCIM_ReceivedData.Code : ERR_NO_ERROR;
		}
	}

	return ERR_TIMEOUT;
}
// ----------------------------------------

Int16U BCCIM_ReadBlock16(pBCCIM_Interface Interface, Int16U Node, Int16U Endpoint)
{
	Int64U timeout;
	BCCIM_ReadBlock16Subfunction(Interface, Node, Endpoint, TRUE);

	timeout = Interface->TimeoutValueTicks + *(Interface->pTimerCounter);
	while(*(Interface->pTimerCounter) < timeout)
	{
		if (BCCIM_ReceivedData.Finished)
		{
			return (BCCIM_ReceivedData.Error) ? BCCIM_ReceivedData.Code : ERR_NO_ERROR;
		}
	}

	return ERR_TIMEOUT;
}
// ----------------------------------------

void BCCIM_ReadBlock16Subfunction(pBCCIM_Interface Interface, Int16U Node, Int16U Endpoint, Boolean Start)
{
	CANMessage message;

	if(Start)
	{
		BCCIM_PrepareDataExchange(CAN_ID_RB_16);
		SavedEndpointRB16 = Endpoint;
		ReadBlock16BufferCounter = 0;
	}

	message.HIGH.WORD.WORD_0 = Endpoint;
	BCCIM_SendFrame(Interface, MBOX_RB_16, &message, Node, CAN_ID_RB_16);
}
// ----------------------------------------

void BCCIM_ReadBlock16Load(pInt16U DataArray, Int16U DataSize, pInt16U DataRead)
{
	Int16U i, Counter = MIN(DataSize, ReadBlock16BufferCounter);

	for (i = 0; i < Counter; ++i)
		DataArray[i] = ReadBlock16Buffer[i];

	*DataRead = Counter;
}
// ----------------------------------------

static void BCCIM_HandleRead16(pBCCIM_Interface Interface)
{
	CANMessage CANInput;
	Interface->IOConfig->IO_GetMessage(MBOX_R_16_A, &CANInput);

	BCCIM_ReceivedData.Address = CANInput.HIGH.WORD.WORD_0;
	BCCIM_ReceivedData.Data = CANInput.HIGH.WORD.WORD_1;
	BCCIM_ReceivedData.Finished = TRUE;
}
// ----------------------------------------

static void BCCIM_HandleWrite16(pBCCIM_Interface Interface)
{
	CANMessage CANInput;
	Interface->IOConfig->IO_GetMessage(MBOX_W_16_A, &CANInput);

	BCCIM_ReceivedData.Address = CANInput.HIGH.WORD.WORD_0;
	BCCIM_ReceivedData.Finished = TRUE;
}
// ----------------------------------------

static void BCCIM_HandleCall(pBCCIM_Interface Interface)
{
	CANMessage CANInput;
	Interface->IOConfig->IO_GetMessage(MBOX_C_A, &CANInput);

	BCCIM_ReceivedData.Action = CANInput.HIGH.WORD.WORD_0;
	BCCIM_ReceivedData.Finished = TRUE;
}
// ----------------------------------------

static void BCCIM_HandleReadBlock16(pBCCIM_Interface Interface)
{
	CANMessage CANInput;

	Interface->IOConfig->IO_GetMessage(MBOX_RB_16_A, &CANInput);

	if(ReadBlock16BufferCounter >= READ_BLOCK_16_BUFFER_SIZE)
	{
		BCCIM_ReceivedData.Finished = TRUE;
		return;
	}

	switch(CANInput.DLC)
	{
		case 8:
			ReadBlock16Buffer[ReadBlock16BufferCounter + 3] = CANInput.LOW.WORD.WORD_3;
		case 6:
			ReadBlock16Buffer[ReadBlock16BufferCounter + 2] = CANInput.LOW.WORD.WORD_2;
		case 4:
			ReadBlock16Buffer[ReadBlock16BufferCounter + 1] = CANInput.HIGH.WORD.WORD_1;
		case 2:
			ReadBlock16Buffer[ReadBlock16BufferCounter] = CANInput.HIGH.WORD.WORD_0;
			ReadBlock16BufferCounter += CANInput.DLC / 2;
			BCCIM_ReadBlock16Subfunction(Interface, CANInput.MsgID.all >> 10, SavedEndpointRB16, FALSE);
			break;
		default:
			BCCIM_ReceivedData.Finished = TRUE;
			break;
	}
}
// ----------------------------------------

static void BCCIM_HandleError(pBCCIM_Interface Interface)
{
	CANMessage CANInput;
	Interface->IOConfig->IO_GetMessage(MBOX_ERR_A, &CANInput);

	BCCIM_ReceivedData.Code = CANInput.HIGH.WORD.WORD_0;
	BCCIM_ReceivedData.Details = CANInput.HIGH.WORD.WORD_1;
	BCCIM_ReceivedData.Error = TRUE;
	BCCIM_ReceivedData.Finished = TRUE;
}
// ----------------------------------------

static void BCCIM_SendFrame(pBCCIM_Interface Interface, Int16U Mailbox, pCANMessage Message, Int32U Node, Int16U Command)
{
	Message->MsgID.all = (Node << 10) | Command;
	Interface->IOConfig->IO_SendMessageEx(Mailbox, Message, TRUE, FALSE);
}
// ----------------------------------------

static void BCCIM_PrepareDataExchange(Int16U Code)
{
	BCCIM_ReceivedData.SavedID = 	Code;
	BCCIM_ReceivedData.Address = 	0;
	BCCIM_ReceivedData.Data = 		0;
	BCCIM_ReceivedData.Action = 	0;
	BCCIM_ReceivedData.Code = 		0;
	BCCIM_ReceivedData.Details = 	0;
	BCCIM_ReceivedData.Finished = 	FALSE;
	BCCIM_ReceivedData.Error = 		FALSE;
}
// ----------------------------------------

// No more.
