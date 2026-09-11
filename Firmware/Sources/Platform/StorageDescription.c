// Header
#include "StorageDescription.h"

// Empty table for now — records will be added in the next step
RecordDescription StorageDescription[] =
{
	{"", DT_Int16U, 0},
};
Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = 0;
