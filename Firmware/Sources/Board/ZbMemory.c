// -----------------------------------------
// Driver for EEPROM via SPI
// ----------------------------------------

// Header
#include "ZbMemory.h"
//
#include "SysConfig.h"


// Constants
//
#define EPROM_DATA_BUFFER_SIZE	16
#define EPROM_DATA_SEGMENT		4		// 4 * 2 + (3) < 16 - SPI FIFO max depth
#define MEM_CL					8
#define EPROM_WRITE_DELAY_US	5000
//
// EPROM commands
#define EPROM_WRITE				0x02
#define EPROM_READ				0x03
#define EPROM_WREN				0x06


// Variables
//
static Int16U EPROMDataBuffer[EPROM_DATA_BUFFER_SIZE];


// Forward functions
//
void ZbMemory_EnableWriteEPROM();


// Functions
//
void ZbMemory_Init()
{
	// Init SPI-D
	ZwSPId_Init(TRUE, MEM_EPROM_BAUDRATE, MEM_CL, MEM_PLR, MEM_PHASE, ZW_SPI_INIT_TX | ZW_SPI_INIT_RX | ZW_SPI_INIT_CS, FALSE, FALSE);
	ZwSPId_InitFIFO(0, 0);
	ZwSPId_ConfigInterrupts(FALSE, FALSE);
	ZwSPId_EnableInterrupts(FALSE, FALSE);
}
// ----------------------------------------

void ZbMemory_WriteValuesEPROM(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize)
{
	Int16U i, j, segCount;

	// Calculate segment count: only 16 SRAM bytes can be written per one transaction (FIFO limit)
	segCount = (BufferSize / EPROM_DATA_SEGMENT) + ((BufferSize % EPROM_DATA_SEGMENT) ? 1 : 0); 
		
	// Write segments
	for(j = 0; j < segCount; ++j)
	{
		Int16U dataSize;
		
		// Calculate address for next segment
		Int16U currentEPROMAddress = EPROMAddress + j * EPROM_DATA_SEGMENT * 2;

		// Enable writing for next operation
		ZbMemory_EnableWriteEPROM();

		// Write command ID
		EPROMDataBuffer[0] = EPROM_WRITE;
		// Memory address
		EPROMDataBuffer[1] = (currentEPROMAddress >> 8);
		EPROMDataBuffer[2] = currentEPROMAddress & 0x00FF;
		// Write data
		for(i = 0; i < MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT); ++i)
		{
			EPROMDataBuffer[3 + i * 2] = Buffer[j * EPROM_DATA_SEGMENT + i] >> 8;
			EPROMDataBuffer[3 + i * 2 + 1] = Buffer[j * EPROM_DATA_SEGMENT + i] & 0x00FF;
		}
	
		// Do SPI communication
		dataSize = 3 + MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT) * 2;
		ZwSPId_Send(EPROMDataBuffer, dataSize, MEM_CL, STTNormal);

		DELAY_US(EPROM_WRITE_DELAY_US);
	}

}
// ----------------------------------------

void ZbMemory_ReadValuesEPROM(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize)
{
	Int16U i, j, segCount, dataSize;

	// Calculate segment count: only 16 FRAM bytes can be read per one transaction (FIFO limit)
	segCount = (BufferSize / EPROM_DATA_SEGMENT) + ((BufferSize % EPROM_DATA_SEGMENT) ? 1 : 0); 
		
	// Read segments
	for(j = 0; j < segCount; ++j)
	{
		// Calculate address for next segment
		Int16U currentEPROMAddress = EPROMAddress + j * EPROM_DATA_SEGMENT * 2;
		
		// Write command ID
		EPROMDataBuffer[0] = EPROM_READ;
		// Memory address
		EPROMDataBuffer[1] = (currentEPROMAddress >> 8);
		EPROMDataBuffer[2] = currentEPROMAddress & 0x00FF;
		
		// Do SPI communication
		dataSize = 3 + MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT) * 2;
		ZwSPId_BeginReceive(EPROMDataBuffer, dataSize, MEM_CL, STTNormal);
		while(ZwSPId_GetWordsToReceive() < dataSize)
			DELAY_US(1);
		ZwSPId_EndReceive(EPROMDataBuffer, dataSize);
		
		// Copy data
		for(i = 0; i < MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT); ++i)
		{
			Int16U result;
			
			// Get data from bytes
			result = (EPROMDataBuffer[3 + i * 2] & 0x00FF) << 8;
			result |= EPROMDataBuffer[3 + i * 2 + 1] & 0x00FF;
			
			Buffer[j * EPROM_DATA_SEGMENT + i] = result;
		}
	}
}
// ----------------------------------------

void ZbMemory_EnableWriteEPROM()
{
	// Write @Enable@ command
	EPROMDataBuffer[0] = EPROM_WREN;
	// Do SPI communication
	ZwSPId_Send(EPROMDataBuffer, 1, MEM_CL, STTNormal);
}
// ----------------------------------------

// No more.
