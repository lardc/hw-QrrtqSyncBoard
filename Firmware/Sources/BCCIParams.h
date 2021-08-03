// -----------------------------------------
// Parameters of BCCI interface
// ----------------------------------------

#ifndef __BCCI_PARAMS_H
#define __BCCI_PARAMS_H

#include "xCCIParams.h"

// Constants
//
#define DEV_ADDR_MPY				(1 << 10)

// Slave params
#define DEVICE_CAN_ADDRESS			10

// Master params
#define BCCI_MASTER_ADDRESS			20
#define BCCIM_ACCEPTANCE_MASK		0xFFFFFC00

#endif // __BCCI_PARAMS_H
