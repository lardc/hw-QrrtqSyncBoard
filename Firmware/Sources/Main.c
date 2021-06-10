// -----------------------------------------
// Program entry point
// ----------------------------------------

// Include
#include <stdinc.h>
//
#include "ZwDSP.h"
#include "ZbBoard.h"
//
#include "SysConfig.h"
//
#include "Controller.h"

// FORWARD FUNCTIONS
// -----------------------------------------
Boolean InitializeCPU();
void InitializeTimers();
void InitializeCAN();
void InitializeSCI();
void InitializeSPI();
void InitializeBoard();
void InitializeADC();
void InitializeController(Boolean GoodClock);
// -----------------------------------------

// FORWARD ISRs
// -----------------------------------------
// CPU External interrupt ISR
ISRCALL XInterrupt_ISR();
// CPU Timer 0 ISR
ISRCALL Timer0_ISR();
// CPU Timer 1 ISR
ISRCALL Timer1_ISR();
// CPU Timer 2 ISR
ISRCALL Timer2_ISR();
// CANa Line 0 ISR
ISRCALL CAN0A_ISR();
// CANb Line 0 ISR
ISRCALL CAN0B_ISR();
// ADC SEQ1 ISR
ISRCALL SEQ1_ISR();
// ILLEGAL ISR
ISRCALL IllegalInstruction_ISR();
// -----------------------------------------

// FUNCTIONS
// -----------------------------------------
void main()
{
	Boolean clockInitResult;

	// Boot process
	clockInitResult = InitializeCPU();

	// PC turn-on patch
	DELAY_US(500000);

	// Only if good clocking was established
	if(clockInitResult)
	{
		InitializeTimers();
		InitializeCAN();
		InitializeSPI();
		InitializeADC();
		InitializeBoard();
	}

	// Try initialize SCI in spite of result of clock initialization
	InitializeSCI();

	// Setup ISRs
	BEGIN_ISR_MAP
		ADD_ISR(TINT0, Timer0_ISR);
		ADD_ISR(TINT1_XINT13, Timer1_ISR);
		ADD_ISR(TINT2, Timer2_ISR);
		ADD_ISR(ECAN0INTA, CAN0A_ISR);
		ADD_ISR(ECAN0INTB, CAN0B_ISR);
		ADD_ISR(XINT1, XInterrupt_ISR);
		ADD_ISR(SEQ1INT, SEQ1_ISR);
	END_ISR_MAP

	// Initialize controller logic
	InitializeController(clockInitResult);
	
	// Only if good clocking was established
	if(clockInitResult)
	{
		// Enable interrupts
		EINT;
		ERTM;

		// Set watch-dog as WDRST
		ZwSystem_SelectDogFunc(FALSE);
		// Enable watch-dog
		ZwSystem_EnableDog(SYS_WD_PRESCALER);
		// Lock WD configuration
		ZwSystem_LockDog();
	
		// Start system ticks timer
		ZwTimer_StartT0();
	}

	// Start update cycle
	ZwTimer_StartT2();

	// Low-priority services
	while(TRUE)
		CONTROL_Idle();
}
// -----------------------------------------

Boolean InitializeCPU()
{
    Boolean clockInitResult;

	// Init clock and peripherals
    clockInitResult = ZwSystem_Init(CPU_PLL, CPU_CLKINDIV, SYS_LOSPCP, SYS_HISPCP, SYS_PUMOD);

    if(clockInitResult)
    {
		// Do default GPIO configuration
		ZwGPIO_Init(GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE);

		// Initialize PIE and vectors
		ZwPIE_Init();
		ZwPIE_Prepare();
    }
   	
	// Config flash
	ZW_FLASH_CODE_SHADOW;
	ZW_FLASH_OPTIMIZE(FLASH_FWAIT, FLASH_OTPWAIT);

   	return clockInitResult;
}
// -----------------------------------------

void InitializeTimers()
{
    ZwTimer_InitT0();
	ZwTimer_SetT0(TIMER0_PERIOD);
	ZwTimer_EnableInterruptsT0(TRUE);

    ZwTimer_InitT1();
	ZwTimer_SetT1(1);	// dummy init
	ZwTimer_EnableInterruptsT1(TRUE);

    ZwTimer_InitT2();
	ZwTimer_SetT2(TIMER2_PERIOD);
	ZwTimer_EnableInterruptsT2(TRUE);
}
// -----------------------------------------

void InitializeSCI()
{
	ZwSCIa_Init(SCIA_BR, SCIA_DB, SCIA_PARITY, SCIA_SB, FALSE);
	ZwSCIa_InitFIFO(16, 0);
	ZwSCIa_EnableInterrupts(FALSE, FALSE);

	ZwSCIb_Init(SCIB_BR, SCIB_DB, SCIB_PARITY, SCIB_SB, FALSE);
	ZwSCIb_InitFIFO(16, 0);
	ZwSCIb_EnableInterrupts(FALSE, FALSE);

	ZwSCI_EnableInterruptsGlobal(FALSE);
}
// -----------------------------------------

void InitializeSPI()
{
	// Common (ABCD)
	ZwSPI_EnableInterruptsGlobal(FALSE);
}
// -----------------------------------------

void InitializeCAN()
{
	// Init CAN
	ZwCANa_Init(CANA_BR, CANA_BRP, CANA_TSEG1, CANA_TSEG2, CANA_SJW);
	ZwCANb_Init(CANB_BR, CANB_BRP, CANB_TSEG1, CANB_TSEG2, CANB_SJW);

	// Register system handler
	ZwCANa_RegisterSysEventHandler(&CONTROL_NotifyCANaFault);
	ZwCANb_RegisterSysEventHandler(&CONTROL_NotifyCANbFault);

    // Allow interrupts for CANa (internal interface)
    ZwCANa_InitInterrupts(TRUE);
    ZwCANa_EnableInterrupts(TRUE);

    // Allow interrupts for CANb (CANopen interface)
	ZwCANb_InitInterrupts(TRUE);
	ZwCANb_EnableInterrupts(TRUE);
}
// -----------------------------------------

void InitializeADC()
{
	// Initialize and prepare ADC
	ZwADC_Init(ADC_PRESCALER, ADC_CD2, ADC_SH);
	ZwADC_ConfigInterrupts(TRUE, FALSE);

	// Enable interrupts on peripheral and CPU levels
	ZwADC_EnableInterrupts(TRUE, FALSE);
	ZwADC_EnableInterruptsGlobal(TRUE);
}
// -----------------------------------------

void InitializeBoard()
{
	// Init DAC
	ZbMemory_Init();
	// Init on-board GPIO
	ZbGPIO_Init();
}
// -----------------------------------------

void InitializeController(Boolean GoodClock)
{
	// Init controllers and logic
	CONTROL_Init(!GoodClock);
}
// -----------------------------------------

// ISRs
// -----------------------------------------
#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(XInterrupt_ISR, "ramfuncs");
	#pragma CODE_SECTION(Timer0_ISR, "ramfuncs");
	#pragma CODE_SECTION(Timer1_ISR, "ramfuncs");
	#pragma CODE_SECTION(Timer2_ISR, "ramfuncs");
	#pragma CODE_SECTION(CAN0A_ISR, "ramfuncs");
	#pragma CODE_SECTION(CAN0B_ISR, "ramfuncs");
	#pragma CODE_SECTION(SEQ1_ISR, "ramfuncs");
	#pragma CODE_SECTION(IllegalInstruction_ISR, "ramfuncs");
#endif
//
#pragma INTERRUPT(Timer0_ISR, HPI);

// External interrupt ISR
ISRCALL XInterrupt_ISR(void)
{
	ZbGPIO_FCROVU_Sync(FALSE);

	XINT_ISR_DONE;
}

// timer 0 ISR
ISRCALL Timer0_ISR(void)
{
	static Int16U LedCounter = 0;

	// Update time
	++CONTROL_TimeCounter;

	// Service watch-dogs
	if (CONTROL_BootLoaderRequest != BOOT_LOADER_REQUEST)
		ZwSystem_ServiceDog();

	++LedCounter;
	if(LedCounter == DBG_COUNTER_PERIOD)
	{
		ZbGPIO_LED_Toggle();
		LedCounter = 0;
	}

	// allow other interrupts from group 1
	TIMER0_ISR_DONE;
}
// -----------------------------------------

// timer 1 ISR
ISRCALL Timer1_ISR(void)
{
	ZwTimer_StopT1();
	ZbGPIO_RCU_Sync(FALSE);
	ZbGPIO_CSU_Sync(FALSE);
	DELAY_US(1);
	ZbGPIO_FCROVU_Sync(TRUE);

	ZwTimer_ReloadT0();
	ZwTimer_ReloadT2();
	ZwTimer_StartT0();
	ZwTimer_StartT2();

	// no PIE
	TIMER1_ISR_DONE;
}
// -----------------------------------------

// timer 2 ISR
ISRCALL Timer2_ISR(void)
{
	// Allow HPI interrupt
	EINT;
	ZwGPIO_WritePin(PIN_LED, TRUE);
	// Invoke control routine
	CONTROL_Update();
	ZwGPIO_WritePin(PIN_LED, FALSE);

	// no PIE
	TIMER2_ISR_DONE;
}
// -----------------------------------------

// Line 0 CANa ISR
ISRCALL CAN0A_ISR(void)
{
    // handle CAN system events
	ZwCANa_DispatchSysEvent();

	// allow other interrupts from group 9
	CAN_ISR_DONE;
}
// -----------------------------------------

// Line 0 CANb ISR
ISRCALL CAN0B_ISR(void)
{
    // handle CAN system events
	ZwCANb_DispatchSysEvent();

	// allow other interrupts from group 9
	CAN_ISR_DONE;
}
// -----------------------------------------

// ADC SEQ1 ISR
ISRCALL SEQ1_ISR(void)
{
	// Handle interrupt
	ZwADC_ProcessInterruptSEQ1();
	// Dispatch results
	ZwADC_Dispatch1();

	// allow other interrupts from group 1
	ADC_ISR_DONE;
}
// -----------------------------------------

// ILLEGAL ISR
ISRCALL IllegalInstruction_ISR(void)
{
	// Disable interrupts
	DINT;

	// Reset system using WD
	ZwSystem_ForceDog();
}
// -----------------------------------------

// No more.
