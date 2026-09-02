#ifndef __DRCU_DICTIONARY_H
#define __DRCU_DICTIONARY_H

// Commands
#define DCRU_ACT_CONFIG						100	// Aplly settings

// Registers
#define DRCU_REG_I_MAX_VALUE				128	// Direct current value A
#define DRCU_REG_I_RATE_INDEX				129	// Номер скорости спада комплекса (0…10)

#define DRCU_REG_CURRENT					202	// Sampled current value

// States
typedef enum __DRCUDeviceState
{
	DRCU_DS_InProcess 	= 5,
	DRCU_DS_ConfigReady = 4,
} DRCUDeviceState;

#endif // __DRCU_DICTIONARY_H
