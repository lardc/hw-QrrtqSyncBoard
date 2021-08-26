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
#define TIMEOUT_HL_LOGIC				60000	// in ms
#define TIMEOUT_HL_LOGIC_SHORT			2000	// in ms
//
#define UNIT_PULSE_TO_PULSE_FAST		700		// in ms
#define UNIT_PULSE_TO_PULSE_PAUSE		1500	// in ms
#define UNIT_PULSE_TO_PULSE_LONG		5000	// in ms
//
#define DELAY_COMMUTATION				500		// in ms
//--------------------------------------------------------

// Fixed current fall rate values
#define CURRENT_FALL_DIDT_010			10
#define CURRENT_FALL_DIDT_015			15
#define CURRENT_FALL_DIDT_020			20
#define CURRENT_FALL_DIDT_050			50
#define CURRENT_FALL_DIDT_100			100
#define CURRENT_FALL_DIDT_150			150
#define CURRENT_FALL_DIDT_200			200
#define CURRENT_FALL_DIDT_300			300
#define CURRENT_FALL_DIDT_500			500
#define CURRENT_FALL_DIDT_600			600
#define CURRENT_FALL_DIDT_1000			1000

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
#define CSU_VOLTAGE_HIGH				1005	// in V x10
#define CSU_VOLTAGE_LOW					995		// in V x10
#define CSU_VOLTAGE_HYST				1		// in V
#define CSU_FAN_TIMEOUT					30000	// in ms
//
#define SYNC_WIDTH_CROVU				300		// in us
//--------------------------------------------------------

#endif // __GLOBAL_H
