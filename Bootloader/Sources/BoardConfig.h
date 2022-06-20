// -----------------------------------------
// Board parameters
// ----------------------------------------

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

// Include
#include <ZwBase.h>

// Program build mode
//
#define BOOT_FROM_FLASH					// normal mode
#define RAM_CACHE_SPI_ABCD				// cache SPI-A(BCD) functions

// Board options
#define OSC_FRQ				(20MHz)			// on-board oscillator
#define CPU_FRQ_MHZ			100				// CPU frequency = 100MHz
#define CPU_FRQ				(CPU_FRQ_MHZ * 1000000L) 
#define SYS_HSP_FREQ		(CPU_FRQ / 2) 	// High-speed bus frequency = 50MHz
#define SYS_LSP_FREQ		(CPU_FRQ / 2) 	// Low-speed bus frequency = 50MHz
//
#define ZW_PWM_DUTY_BASE	5000

// Peripheral options
#define HWUSE_SCI_A

// IO debug LED
#define DBG_LED				34

// IO placement
#define SCI_A_QSEL			GPAQSEL2
#define SCI_A_MUX			GPAMUX2
#define SCI_A_TX			GPIO29
#define SCI_A_RX			GPIO28
#define SCI_A_MUX_SELECTOR	1

// GPIO pins
#define PIN_CSU_PS			0
#define PIN_SWITCH_DUT		13
#define PIN_CONTROL_DUT		9
#define PIN_LED				34
#define PIN_LED2			33
#define PIN_SYNC_RCU		2
#define PIN_SYNC_DCU		6
#define PIN_SYNC_FCROVU		12
#define PIN_CSU_FAN			17
#define PIN_CSU_SYNC		10
#define PIN_CSU_DISCH		32
#define PIN_SYNC_SCOPE		22
#define PIN_SYNC_SCOPE2		23
#define PIN_PC_PWR			11
#define PIN_DBG				14
#define PIN_DCU_READY		15
#define PIN_CSU_ITRIG		25

#endif // __BOARD_CONFIG_H
