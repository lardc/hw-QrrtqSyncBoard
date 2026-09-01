// -----------------------------------------
// Constraints for tunable parameters
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
//
#include "DataTable.h"
#include "Global.h"

// Types
//
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;


// Restrictions
//
#define X_D_DEF0					10
#define X_D_DEF1					100
#define X_D_DEF2					1000
#define X_D_DEF3					10000

// in us/10

#define MAX_RCU_TRIG_OFFSET			27000
#define MAX_GATE_DRV_TURN_ON_DELAY	500

// in us/10
#define MAX_FCROVU_TRIG_OFFSET		500

// in us
#define MAX_TQ_OFFSET				15

#define TQ_SINGLE_PULSE_MIN			3
#define TQ_SINGLE_PULSE_MAX			1000
#define TQ_SINGLE_PULSE_DEF			100

// DCU/RCU
//

#define MAX_UNIT_DRCU				6
#define DEF_UNIT_DRCU				6

#define DCU_I_RISE_RATE_MAX			300		// in A/us x100
#define DCU_I_RISE_RATE_DEF			66		// in A/us x100

#define DIRECT_CURRENT_MIN			100		// in A
#define DIRECT_CURRENT_MAX			3300	// in A
#define DIRECT_CURRENT_DEF			300		// in A

#define REVERSE_CURRENT_MIN         0      // в А
#define REVERSE_CURRENT_MAX         3300    // в А
#define REVERSE_CURRENT_DEF         0     // в А

#define IDC_PULSE_WIDTH_MIN			100		// in us
#define IDC_PULSE_WIDTH_MAX			5000	// in us
#define IDC_PULSE_WIDTH_DEF			2000	// in us

#define IRC_SYNC_WIDTH_MIN			500		// in us
#define IRC_SYNC_WIDTH_MAX 			2000	// in us
#define IRC_SYNC_WIDTH_DEF			500		// in us

#define REVERSE_RATE_MIN			0		// in A/us x10
#define REVERSE_RATE_MAX			10		// in A/us x10
#define REVERSE_RATE_DEF			4		// in A/us x10

#define FALL_RATE_MIN				1		// A/мкс x10
#define MAX_FALL_RATE				1000	// A/мкс x10
#define FALL_RATE_R0_DEF			10		// A/мкс x10
#define FALL_RATE_R1_DEF			15		// A/мкс x10
#define FALL_RATE_R2_DEF			20		// A/мкс x10
#define FALL_RATE_R3_DEF			50		// A/мкс x10
#define FALL_RATE_R4_DEF			100		// A/мкс x10
#define FALL_RATE_R5_DEF			150		// A/мкс x10
#define FALL_RATE_R6_DEF			200		// A/мкс x10
#define FALL_RATE_R7_DEF			300		// A/мкс x10
#define FALL_RATE_R8_DEF			500		// A/мкс x10
#define FALL_RATE_R9_DEF			600		// A/мкс x10
#define FALL_RATE_R10_DEF			1000	// A/мкс x10
//
#define I_TO_V_OFFSET_DEF			0
#define K_COEF_MIN					1
#define I_TO_V_K_DEF				1
#define I_TO_V_K2_DEF				0
#define CTRL_OFFSET_DEF				0
#define CTRL_K_DEF					1000
#define I_TO_DAC_P0_DEF				0
#define I_TO_DAC_P1_DEF				1000
#define I_TO_DAC_P2_DEF				1000
// FCROVU
//
#define OS_VOLATGE_MIN				400		// in V
#define OS_VOLATGE_MAX				4400	// in V
#define OS_VOLATGE_DEF				500		// in V

#define OSV_RATE_MIN				20		// in V/us
#define OSV_RATE_MAX				200		// in V/us
#define OSV_RATE_DEF				50		// in V/us

#define OSV_CURRENT_MIN				0		// in A
#define OSV_CURRENT_MAX				1600	// in A
#define OSV_CURRENT_DEF				0       // in A

// CSU
//
#define CSU_VOLTAGE_MIN				500		// in V x10
#define CSU_VOLTAGE_MAX				1000	// in V x10
#define CSU_VOLTAGE_DEF				900 	// in V x10

#define CSU_VOLTAGE_HYST_MIN		1		// in V x10
#define CSU_VOLTAGE_HYST_MAX		50		// in V x10
#define CSU_VOLTAGE_HYST_DEF		5		// in V x10
//

#define PULSE_TO_PULSE_DEF          5000    // in ms

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
