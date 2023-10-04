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

// in ns
#define MAX_RCU_TRIG_OFFSET			27000
#define MAX_GATE_DRV_TURN_ON_DELAY	500

// in us
#define TQ_SINGLE_PULSE_MIN			5
#define TQ_SINGLE_PULSE_MAX			1000
#define TQ_SINGLE_PULSE_DEF			100

// DCU/RCU
//

#define MAX_UNIT_DRCU				6
#define DEF_UNIT_DRCU				6

#define MAX_RCU_SYNC				7000	// â ìêñ
#define DEF_RCU_SYNC				2000	// â ìêñ

#define DCU_I_RISE_RATE_MAX			300		// in A/us x100
#define DCU_I_RISE_RATE_DEF			66		// in A/us x100

#define DIRECT_CURRENT_MIN			100		// in A
#define DIRECT_CURRENT_MAX			3300	// in A
#define DIRECT_CURRENT_DEF			300		// in A

#define IDC_PULSE_WIDTH_MIN			500		// in us
#define IDC_PULSE_WIDTH_MAX			5000	// in us
#define IDC_PULSE_WIDTH_DEF			2000	// in us

#define IRC_SYNC_WIDTH_MIN			500		// in us
#define IRC_SYNC_WIDTH_MAX 			2000	// in us
#define IRC_SYNC_WIDTH_DEF			500		// in us

#define REVERSE_RATE_MIN			0		// in A/us x10
#define REVERSE_RATE_MAX			10		// in A/us x10
#define REVERSE_RATE_DEF			4		// in A/us x10

#define MAX_FALL_RATE				1000	// A/ìêñ x10
#define DEF_FALL_RATE_R0			10		// A/ìêñ x10
#define DEF_FALL_RATE_R1			15		// A/ìêñ x10
#define DEF_FALL_RATE_R2			20		// A/ìêñ x10
#define DEF_FALL_RATE_R3			50		// A/ìêñ x10
#define DEF_FALL_RATE_R4			100		// A/ìêñ x10
#define DEF_FALL_RATE_R5			150		// A/ìêñ x10
#define DEF_FALL_RATE_R6			200		// A/ìêñ x10
#define DEF_FALL_RATE_R7			300		// A/ìêñ x10
#define DEF_FALL_RATE_R8			500		// A/ìêñ x10
#define DEF_FALL_RATE_R9			600		// A/ìêñ x10
#define DEF_FALL_RATE_R10			1000	// A/ìêñ x10
//
#define DEF_I_TO_V_OFFSET			0
#define DEF_I_TO_V_K				1000
#define DEF_I_TO_V_K2				0
#define DEF_CTRL1_OFFSET			0
#define DEF_CTRL1_K					1000
// FCROVU
//
#define OS_VOLATGE_MIN				100		// in V
#define OS_VOLATGE_MAX				2000	// in V
#define OS_VOLATGE_DEF				200		// in V

#define OSV_RATE_MIN				10		// in V/us
#define OSV_RATE_MAX				2000	// in V/us
#define OSV_RATE_DEF				20		// in V/us

#define OSV_CURRENT_MIN				400		// in A
#define OSV_CURRENT_MAX				1600	// in A
#define OSV_CURRENT_DEF				1600	// in A

// CSU
//
#define CSU_VOLTAGE_MIN				100		// in V x10
#define CSU_VOLTAGE_MAX				1000	// in V x10
#define CSU_VOLTAGE_DEF				500 	// in V x10

#define CSU_VOLTAGE_HYST_MIN		1		// in V x10
#define CSU_VOLTAGE_HYST_MAX		50		// in V x10
#define CSU_VOLTAGE_HYST_DEF		5		// in V x10

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
