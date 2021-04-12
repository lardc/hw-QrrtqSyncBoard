// -----------------------------------------
// Board parameters
// ----------------------------------------

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

// Include
#include <ZwBase.h>

// Program build mode
//
#define BOOT_FROM_FLASH						// normal mode
#define RAM_CACHE_SPI_X						// cache SPI-A(BCD) functions

// Board options
#define DSP28_2809							// on-board CPU
#define OSC_FRQ				(20MHz)			// on-board oscillator
#define CPU_FRQ_MHZ			100				// CPU frequency = 100MHz
#define CPU_FRQ				(CPU_FRQ_MHZ * 1000000L)
#define SYS_HSP_FREQ		(CPU_FRQ / 2) 	// High-speed bus frequency
#define SYS_LSP_FREQ		(CPU_FRQ / 4) 	// Low-speed bus frequency
//
#define ZW_PWM_DUTY_BASE	1024

// Peripheral options
#define HWUSE_SPI_D
#define HWUSE_SCI_A
#define HWUSE_SCI_B

// IO placement
//
#define SPI_D_QSEL			GPAQSEL1
#define SPI_D_MUX			GPAMUX1
#define SPI_D_SIMO			GPIO1
#define SPI_D_SOMI			GPIO3
#define SPI_D_CLK			GPIO5
#define SPI_D_CS			GPIO7
//
#define SCI_A_QSEL			GPAQSEL2
#define SCI_A_MUX			GPAMUX2
#define SCI_A_TX			GPIO29
#define SCI_A_RX			GPIO28
#define SCI_A_MUX_SELECTOR	1
//
#define SCI_B_QSEL			GPAQSEL2
#define SCI_B_MUX			GPAMUX2
#define SCI_B_TX			GPIO18
#define SCI_B_RX			GPIO19
#define SCI_B_MUX_SELECTOR	2
//
#define PIN_SWITCH_HV_IGBT	2
#define PIN_SYNC_SCOPE		4
#define PIN_SWITCH_REV_CUR	6
#define PIN_SWITCH_DIR_CUR	8
#define PIN_SYNC_FCROVU		10
#define PIN_SWITCH_DUT		11
#define PIN_DC_READY		13
#define PIN_LED_1			33
#define PIN_LED_2			34

#endif // __BOARD_CONFIG_H
