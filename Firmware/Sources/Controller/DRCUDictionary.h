#ifndef __DRCU_DICTIONARY_H
#define __DRCU_DICTIONARY_H

// Commands
#define DCRU_ACT_CONFIG						101	// Aplly settings

// Registers
#define DRCU_REG_I_MAX_VALUE				129	// Direct current value A
#define DRCU_REG_I_RATE_RISE				130	// Direct current rise rate A/us
#define DRCU_REG_I_RATE_FALL				131	// Direct current fall rate A/us

#define DRCU_REG_CURRENT					200	// Sampled current value

// States
typedef enum __DRCUDeviceState
{
	DRCU_DS_Charging = 5,
	DRCU_DS_ConfigReady = 6,
} DRCUDeviceState;

#endif // __DRCU_DICTIONARY_H
