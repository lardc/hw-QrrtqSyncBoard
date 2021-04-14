#ifndef __DRCU_DICTIONARY_H
#define __DRCU_DICTIONARY_H

// Commands
#define DCRU_ACT_CONFIG						101	// Aplly settings

// Registers
#define DRCU_REG_I_MAX_VALUE				129	// Direct current value A
#define DRCU_REG_I_RATE_RISE				130	// Direct current rise rate A/us
#define DRCU_REG_I_RATE_FALL				131	// Direct current fall rate A/us

// States
typedef enum __DRCUDeviceState
{
	DRCU_DS_None = 0,
	DRCU_DS_Fault = 1,
	DRCU_DS_Disabled = 2,
	DRCU_DS_Charging = 3,
	DRCU_DS_Ready = 4,
	DRCU_DS_ConfigReady = 5,
	DRCU_DS_InProcess = 6
} DRCUDeviceState;

#endif // __DRCU_DICTIONARY_H
