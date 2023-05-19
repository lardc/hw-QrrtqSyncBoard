// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

// Include
#include "stdinc.h"
#include "SysConfig.h"

// Global parameters
//--------------------------------------------------------
//
// Password to unlock non-volatile area for write
#define ENABLE_LOCKING					FALSE
#define UNLOCK_PWD_1					1
#define UNLOCK_PWD_2					1
#define UNLOCK_PWD_3					1
#define UNLOCK_PWD_4					1
//
#define EP_COUNT						12
#define VALUES_x_SIZE					2000
//
#define	SCCI_MASTER_TIMEOUT_TICKS_MS	500		// in ms
#define	SCCI_SLAVE_TIMEOUT_TICKS_MS		100		// in ms
#define	BCCI_MASTER_TIMEOUT_TICKS_MS	1000	// in ms
//
#define	NODEID_SCCI_SCOPE				0
//
#define TIMEOUT_REINIT_RS232			10000	// in ms
#define TIMEOUT_HL_LOGIC				10000	// in ms
#define TIMEOUT_HL_LOGIC_POWER_ON		300000	// in ms
#define TIMEOUT_HL_LOGIC_SHORT			2000	// in ms
//
#define UNIT_PULSE_TO_PULSE_FAST		700		// in ms
#define UNIT_PULSE_TO_PULSE_PAUSE		1500	// in ms
#define UNIT_PULSE_TO_PULSE_LONG		5000	// in ms
//
#define DELAY_COMMUTATION				500		// in ms
//--------------------------------------------------------

// Fixed current fall rate values
#define CURRENT_FALL_DIDT_R0			0
#define CURRENT_FALL_DIDT_R1			1
#define CURRENT_FALL_DIDT_R2			2
#define CURRENT_FALL_DIDT_R3			3
#define CURRENT_FALL_DIDT_R4			4
#define CURRENT_FALL_DIDT_R5			5
#define CURRENT_FALL_DIDT_R6			6
#define CURRENT_FALL_DIDT_R7			7
#define CURRENT_FALL_DIDT_R8			8
#define CURRENT_FALL_DIDT_R9			9
#define CURRENT_FALL_DIDT_R10			10

// Misc
//--------------------------------------------------------
#define UNIT_TQ_MEASURE_PULSES			11		// (in pulses) + 1
#define UNIT_TQ_EXTRA_PULSES			15		// in pulses
#define QRR_AVG_COUNTER					1		// in pulses
#define UNIT_MAX_NUM_OF_PULSES			(UNIT_TQ_MEASURE_PULSES + UNIT_TQ_EXTRA_PULSES)	// in pulses
#define TQ_MAX_TIME						1000	// in us
#define TQ_FIRST_PROBE					100		// in us
#define TQ_ZERO_OFFSET					3		// in us
//
#define OSV_ON_TIME						500		// in us
#define OSV_ON_TIME_TICK				(OSV_ON_TIME / TIMER2_PERIOD)
//
#define DC_READY_TIMEOUT_DELAY			10000	// in us
#define DC_READY_TIMEOUT_TICK			(DC_READY_TIMEOUT_DELAY / TIMER2_PERIOD)
#define DC_DRIVER_OFF_DELAY_MIN			200		// in us
//
#define RT_DATA_READ_DELAY				500000	// in us
#define RT_DATA_READ_DELAY_TICK			(RT_DATA_READ_DELAY / TIMER2_PERIOD)
//
#define DRCU_CURRENT_MAX				1100	// in A
#define CSU_ROUGH_VOLTAGE_HYST			10		// in V x10
#define CSU_FAN_TIMEOUT					30000	// in ms
//
#define SYNC_WIDTH_CROVU				300		// in us
//--------------------------------------------------------

#endif // __GLOBAL_H
