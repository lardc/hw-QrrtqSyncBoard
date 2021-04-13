// -----------------------------------------
// System parameters
// ----------------------------------------
#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

// Include
#include <ZwBase.h>
#include <BoardConfig.h>

// CPU & System
//--------------------------------------------------------
#define CPU_PLL				10          // OSCCLK * PLL div 2 = CPUCLK: 20 * 10 / 2 = 100
#define CPU_CLKINDIV		0           // "div 2" in previous equation
#define SYS_HISPCP       	0x01   		// SYSCLKOUT / 2
#define SYS_LOSPCP       	0x02    	// SYSCLKOUT / 4
//--------------------------------------------------------

// Boot-loader
//--------------------------------------------------------
#define BOOT_LOADER_REQUEST	0xABCD
//--------------------------------------------------------

// Power control
//--------------------------------------------------------
#define SYS_PUMOD			ZW_POWER_SPID_CLK | \
							ZW_POWER_SCIA_CLK | ZW_POWER_SCIB_CLK | \
							ZW_POWER_CANA_CLK | ZW_POWER_CANB_CLK

#define SYS_WD_PRESCALER	0x07
//--------------------------------------------------------

// ADC
//--------------------------------------------------------
#define ADC_PRESCALER		0			// HSPCLK / (1 * 1) = 50 MHz
#define ADC_CD2				TRUE		// Div ADC core / 2 = 25MHz
#define ADC_SH				0			// S/H sample window = 1 => 12.5 MSPS
//--------------------------------------------------------

// GPIO
//--------------------------------------------------------
// Input filters
#define GPIO_TSAMPLE		5			// T[sample_A] = (1/ 100MHz) * (2 * 5) = 100 nS
#define GPIO_NSAMPLE		3			// 6 samples: T = 100nS * 3 = 300 nS

// Flash
//--------------------------------------------------------
#define FLASH_FWAIT			3
#define FLASH_OTPWAIT		5
//--------------------------------------------------------

// TIMERs
//--------------------------------------------------------
#define CS_T0_FREQ			1000		// 1000 Hz
#define CS_T2_FREQ			10000		// 10 kHz

#define TIMER0_PERIOD		(1000000L / CS_T0_FREQ)
#define TIMER2_PERIOD		(1000000L / CS_T2_FREQ)

#define DBG_FREQ			2			// 2 Hz
#define DBG_COUNTER_PERIOD	(CS_T0_FREQ / (DBG_FREQ * 2))
//--------------------------------------------------------

// EPROM
//--------------------------------------------------------
#define MEM_EPROM_BAUDRATE	1562500L	// SPI clock = 1.5625 MHz
#define MEM_CL				8 			// 8-bit element width
#define MEM_PLR				FALSE		// CLK low in idle state (SPI MODE 0)
#define MEM_PHASE			TRUE		// data output on rising edge (SPI MODE 0)
//--------------------------------------------------------

// SCI-A
//--------------------------------------------------------
#define SCIA_BR				115200L		// UART baudrate
#define SCIA_DB				8			// 8 bit
#define SCIA_SB				FALSE		// 1 stop bit
#define SCIA_PARITY			ZW_PAR_NONE	// No parity
//--------------------------------------------------------

// SCI-B
//--------------------------------------------------------
#define SCIB_BR				115200L		// UART baudrate
#define SCIB_DB				8			// 8 bit
#define SCIB_SB				FALSE		// 1 stop bit
#define SCIB_PARITY			ZW_PAR_NONE	// No parity
//--------------------------------------------------------

// CAN-A
//--------------------------------------------------------
#define CANA_BR				1000000L
#define CANA_BRP			9
#define CANA_TSEG1			6
#define CANA_TSEG2			1
#define CANA_SJW			1
//--------------------------------------------------------

// CAN-B
//--------------------------------------------------------
#define CANB_BR				1000000L
#define CANB_BRP			9
#define CANB_TSEG1			6
#define CANB_TSEG2			1
#define CANB_SJW			1
//--------------------------------------------------------

#endif // __SYSCONFIG_H
