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
#define MAX_RCU_TRIG_OFFSET			20000
#define MAX_GATE_DRV_TURN_ON_DELAY	500

// in us
#define TQ_SINGLE_PULSE_MIN			5
#define TQ_SINGLE_PULSE_MAX			1000
#define TQ_SINGLE_PULSE_DEF			100

// DCU/RCU
//
#define DCU_I_RISE_RATE_MAX			300		// in A/us x100
#define DCU_I_RISE_RATE_DEF			66		// in A/us x100

#define DIRECT_CURRENT_MIN			100		// in A
#define DIRECT_CURRENT_MAX			500		// in A
#define DIRECT_CURRENT_DEF			100		// in A

#define IDC_PULSE_WIDTH_MIN			500		// in us
#define IDC_PULSE_WIDTH_MAX			5000	// in us
#define IDC_PULSE_WIDTH_DEF			2000	// in us

#define REVERSE_RATE_MIN			10		// in A/us x10
#define REVERSE_RATE_MAX			1000	// in A/us x10
#define REVERSE_RATE_DEF			100		// in A/us x10

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

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
