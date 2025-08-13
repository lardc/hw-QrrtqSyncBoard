// -----------------------------------------
// Parameters of BCCI interface
// ----------------------------------------

#ifndef __BCCI_PARAMS_H
#define __BCCI_PARAMS_H

#include "xCCIParams.h"

// Constants
//
#define CAN_SLAVE_NID_MPY			(1ul << 10)

// Slave params
#define DEVICE_CAN_ADDRESS			10

// Master params
#define BCCI_MASTER_ADDRESS			20
#define BCCIM_ACCEPTANCE_MASK		0xFFFFFC00

#define CAN_MASTER_NID_MASK			0x03FC0000
#define CAN_SLAVE_NID_MASK			0x0003FC00

#define CAN_MASTER_NID_MPY			(1ul << 18)

#endif // __BCCI_PARAMS_H
