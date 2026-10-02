// Header
#include "StorageDescription.h"
#include "Global.h"
#include "Logic.h"

// Variables
RecordDescription StorageDescription[] =
{
	{"REG_DEV_STATE",							DT_Int16U, 1},
	{"REG_FAULT_REASON",						DT_Int16U, 1},
	{"REG_DISABLE_REASON",						DT_Int16U, 1},
	{"REG_WARNING",								DT_Int16U, 1},
	{"REG_PROBLEM",								DT_Int16U, 1},
	{"REG_FAULT_REASON_EXT",					DT_Int16U, 1},
	{"REG_FINISHED",							DT_Int16U, 1},
	{"REG_PULSES_COUNTER",						DT_Int16U, 1},
	{"REG_LOGIC_STATE",							DT_Int16U, 1},
	{"LOGIC_ExtDeviceState",					DT_Int16U, sizeof(ExternalDeviceState) / sizeof(Int16U)},
	{"EP 3 DIAG1 DevTrig",						DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 4 DIAG2 OSVTime",						DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 5 DIAG3 Irr",							DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 6 DIAG4 Trr",							DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 7 DIAG5 Qrr",							DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 8 DIAG6 Idc",							DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 9 DIAG7 ZeroI",						DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 10 DIAG8 ZeroV",						DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
	{"EP 11 DIAG9 dIdt",						DT_Int16U, UNIT_MAX_NUM_OF_PULSES},
};
Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);
