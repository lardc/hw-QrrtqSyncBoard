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

// QPU LP
//
#define DIRECT_CURRENT_MIN			10		// in A
#define DIRECT_CURRENT_MAX			500		// in A
#define DIRECT_CURRENT_DEF			200		// in A

#define IDC_PULSE_WIDTH_MIN			500		// in us
#define IDC_PULSE_WIDTH_MAX			5000	// in us
#define IDC_PULSE_WIDTH_DEF			2000	// in us

#define IDC_RISE_RATE_MIN			1		// in A/us x10
#define IDC_RISE_RATE_MAX			50		// in A/us x10
#define IDC_RISE_RATE_DEF			5		// in A/us x10

#define REVERSE_RATE_MIN			10		// in A/us x10
#define REVERSE_RATE_MAX			1000	// in A/us x10
#define REVERSE_RATE_DEF			100		// in A/us x10

// FCROVU
//
#define OS_VOLATGE_MIN				200		// in V
#define OS_VOLATGE_MAX				1900	// in V
#define OS_VOLATGE_DEF				200		// in V

#define OSV_RATE_MIN				20		// in V/us
#define OSV_RATE_MAX				200		// in V/us
#define OSV_RATE_DEF				20		// in V/us

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
