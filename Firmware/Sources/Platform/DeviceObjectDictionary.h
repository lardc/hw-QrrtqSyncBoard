// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// ACTIONS
//
#define ACT_ENABLE_POWER			1	// Enable flyback converter
#define ACT_DISABLE_POWER			2	// Disable flyback converter
#define ACT_CLR_FAULT				3	// Clear fault (try switch state from FAULT to NONE)
#define ACT_CLR_WARNING				4	// Clear warning
//
#define ACT_DIAG_READ_REG			10	// Read ext-node register
#define ACT_DIAG_WRITE_REG			11	// Write ext-node register
#define ACT_DIAG_CALL				12	// Call ext-node action
#define ACT_DIAG_READ_EP			13	// Read ext-node EP array
#define ACT_DIAG_TURN_ON_PC			14	// Turn-on PC
#define ACT_DIAG_FILL_RESULTS		15	// Fill results with values
//
#define ACT_DIAG_PULSE_DUT			20	// Generate control pulse for DUT
#define ACT_DIAG_PULSE_SW_DUT		21	// Generate control pulse for switch DUT
#define ACT_DIAG_PULSE_DC_SYNC		22	// Generate control pulse for direct current sync
#define ACT_DIAG_PULSE_RC_SYNC		23	// Generate control pulse for reverse current sync
#define ACT_DIAG_PULSE_CSU_PS		24	// Generate control pulse for CSU power supply
#define ACT_DIAG_PULSE_CSU_DISCH	25	// Generate control pulse for CSU discharge
#define ACT_DIAG_PULSE_CSU_FAN		26	// Generate control pulse for CSU fan
#define ACT_DIAG_PULSE_CSU_SYNC		27	// Generate control pulse for CSU sync
#define ACT_DIAG_PULSE_SCOPE		28	// Generate control pulse for high speed scope
#define ACT_DIAG_PULSE_FCROVU		29	// Generate control pulse for FCROVU
#define ACT_DIAG_PULSE_SB			30	// Generate control pulse for SensingBoard
#define ACT_DIAG_PULSE_QCUHC		31	// Generate control pulse for QCUHC
#define ACT_DIAG_PULSE_GATE_RELAY	32	// Generate control pulse for Gate relay (QCUHV)
#define ACT_DIAG_QRR_PULSE			33
//
#define ACT_START					100	// Start measurement
#define ACT_STOP					101	// Force measurement stop
#define ACT_SINGLE_START			102	// Start single-pulse measurement
#define ACT_COMMUTATION_FORCED_ON	110	// Commutation enable
#define ACT_COMMUTATION_FORCED_OFF	111	// Commutation disable
//
#define ACT_SAVE_TO_ROM				200	// Save parameters to EEPROM module
#define ACT_RESTORE_FROM_ROM		201	// Restore parameters from EEPROM module
#define ACT_RESET_TO_DEFAULT		202	// Reset parameters to default values (only in controller memory)
#define ACT_LOCK_NV_AREA			203	// Lock modifications of parameters area
#define ACT_UNLOCK_NV_AREA			204	// Unlock modifications of parameters area (password-protected)
//
#define ACT_BOOT_LOADER_REQUEST		320	// Request reboot to bootloader
//
// ----------------------------------------

// REGISTERS
//
#define REG_EMULATE_CROVU			0	// Enable CROVU emulation
#define REG_EMULATE_FCROVU			1	// Enable FCROVU emulation
#define REG_EMULATE_DCU1			2	// Enable DCU1 emulation
#define REG_EMULATE_DCU2			3	// Enable DCU2 emulation
#define REG_EMULATE_DCU3			4	// Enable DCU3 emulation
#define REG_EMULATE_RCU1			5	// Enable RCU1 emulation
#define REG_EMULATE_RCU2			6	// Enable RCU2 emulation
#define REG_EMULATE_RCU3			7	// Enable RCU3 emulation
#define REG_EMULATE_SCOPE			8	// Enable SCOPE emulation
#define REG_EMULATE_CSU				9	// Enable CSU emulation
//
#define REG_CROVU_NODE_ID			10	// CROVU CAN node ID
#define REG_FCROVU_NODE_ID			11	// FCROVU CAN node ID
#define REG_DCU1_NODE_ID			12	// DCU1 CAN node ID
#define REG_DCU2_NODE_ID			13	// DCU2 CAN node ID
#define REG_DCU3_NODE_ID			14	// DCU3 CAN node ID
#define REG_RCU1_NODE_ID			15	// RCU1 CAN node ID
#define REG_RCU2_NODE_ID			16	// RCU2 CAN node ID
#define REG_RCU3_NODE_ID			17	// RCU3 CAN node ID

#define REG_RR_STOP_RCU_EN			18	// Разрешить остановку RCU в точке обратного восстановления
#define REG_RR_STOP_RCU_TICKS		19	// Задержка до остановки в тиках таймера 2

#define REG_SAFETY_EN				20	// Safety enable
#define REG_CSU_VOLTAGE_K			21 	// CSU voltage coefficient
#define REG_DCU_CURRENT_PER_UNIT	22	// Максимальный ток DCU на блок (в А)
#define REG_RCU_CURRENT_PER_UNIT	23	// Максимальный ток RCU на блок (в А)
#define REG_RCU_DEF_FALL_RATE		24	// Скорость спада тока RCU на блок (в А/мкс х10)
#define REG_DCU_I_RISE_RATE			25	// Скорость нарастания тока в DCU (в А/мкс x10)
#define REG_GATE_DRV_TURNON_DELAY	26	// Задержка формирования импульса управления (в мкс)
#define REG_CSU_VOLTAGE_OFFSET		27	// Оффсет при пересчете напряжения CSU
#define REG_UNIT_DRCU				28  // Количество блоков в комплексе
#define REG_RCU_SYNC_MAX			29	// Максимальная длительности синхросигнала RCU
//
#define REG_RCU_TOFFS_R0			30	// Сдвиг срабатывания синхронизации RCU при 0 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R1			31	// Сдвиг срабатывания синхронизации RCU при 1 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R2			32	// Сдвиг срабатывания синхронизации RCU при 2 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R3			33	// Сдвиг срабатывания синхронизации RCU при 3 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R4			34	// Сдвиг срабатывания синхронизации RCU при 4 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R5			35	// Сдвиг срабатывания синхронизации RCU при 5 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R6			36	// Сдвиг срабатывания синхронизации RCU при 6 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R7			37	// Сдвиг срабатывания синхронизации RCU при 7 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R8			38	// Сдвиг срабатывания синхронизации RCU при 8 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R9   			39	// Сдвиг срабатывания синхронизации RCU при 9 скорости спада (в нс x10)
#define REG_RCU_TOFFS_R10			40	// Сдвиг срабатывания синхронизации RCU при 10 скорости спада (в нс x10)
#define REG_RCU_TOFFS_K4			41	// коэффицент сдвига
//
#define REG_FALL_RATE_R0			50 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R1			51 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R2			52 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R3			53 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R4			54 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R5			55 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R6			56 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R7			57 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R8			58 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R9			59 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
#define REG_FALL_RATE_R10			60 	// Фактическая скорость спада при 0 скорости спада комплекса (в А/мкс)
//
#define REG_I_TO_V_OFFSET			70  // Добавочное смещение преобразования тока в напряжение внутреннего источника
#define REG_I_TO_V_K				71	// Добавочный коэффицент преобразования тока в напряжение внутреннего источника
#define REG_I_TO_V_K2				72	// Добавочный квадратичный коэффициент преобразования тока в напряжение внутреннего источника
#define REG_CTRL1_OFFSET			73	// Добавочное смещение грубой подстройки тока при скрости спада
#define REG_CTRL1_K					74	// Добавочный коэффицент грубой подстройки тока при скрости спада
// ----------------------------------------
//
#define REG_MODE					128	// Measurement mode
#define REG_DIRECT_CURRENT			129	// Direct current amplitude (in A)
#define REG_DCU_PULSE_WIDTH			130	// Direct current pulse duration (in us)
#define REG_RCU_SYNC_WIDTH			131	// Длина импульса синхронизации RCU
#define REG_CURRENT_FALL_RATE		132 // Direct current fall rate (in A/us x10)
#define REG_OFF_STATE_VOLTAGE		133 // Off-state voltage (in V)
#define REG_OSV_RATE				134 // Off-state voltage rise rate (in V/us)
#define REG_TRIG_TIME				135	// Time to apply on-state voltage in single-pulse mode (in us)
#define REG_TRR_DETECTION_MODE		136	// Select trr detection mode (0 - 90%-25%; 1 - 90%-50%)
#define REG_FCROVU_I_SHORT			137	// FCROVU short circuit current
#define REG_CSU_VOLTAGE_THRE		138	// CSU voltage set (in V x10)
#define REG_CSU_VOLTAGE_HYST		139 // CSU voltage hysteresis (in V x10)
//
#define REG_DIAG_NID				150	// Node-id diagnostic register
#define REG_DIAG_IN_1				151	// Input diagnostic register 1
#define REG_DIAG_IN_2				152	// Input diagnostic register 2
#define REG_DIAG_DISABLE_PLOT_READ	153	// Disable plot read from scope
//
#define REG_DCU_SYNC_DELAY			160
#define REG_RCU_SYNC_DELAY			161
//
#define REG_DBG						170	// Отладочный регистр
#define REG_DBG2					171	// Отладочный регистр 2
#define REG_DBG3					172	// Отладочный регистр 3(DC_CurrentZeroPoint)
//
#define REG_PWD_1					180	// Unlock password location 1
#define REG_PWD_2					181	// Unlock password location 2
#define REG_PWD_3					182	// Unlock password location 3
#define REG_PWD_4					183	// Unlock password location 4
//
#define REG_SP__2					191
//
// ----------------------------------------
//
#define REG_DEV_STATE				192	// Device state
#define REG_FAULT_REASON			193	// Fault reason in the case DeviceState -> FAULT
#define REG_DISABLE_REASON			194	// Disbale reason in the case DeviceState -> DISABLE
#define REG_WARNING					195	// Warning if present
#define REG_PROBLEM					196	// Problem if present
#define REG_FAULT_REASON_EXT		197	// Fault reason extended code
#define REG_FINISHED				198	// Indicates that test is done and there is result or fault
#define REG_PULSES_COUNTER			199	// Pulses counter while system is InProcess
//
#define REG_LOGIC_STATE				200	// High-level logic state
#define REG_SLAVE_DEVICE			201	// Slave device
#define REG_SLAVE_FUNC				202	// Slave function
#define REG_SLAVE_ERR				203	// Slave error
#define REG_SLAVE_EXTDATA			204	// Slave call extended data
//
#define REG_RES_QRR					210	// Reverse recovery charge (in uC x10) (Calculation defined by GOST)
#define REG_RES_IRR					211	// Reverse recovery current amplitude (in A)
#define REG_RES_TRR					212	// Reverse recovery time (in us x10)
#define REG_RES_TQ					213	// Turn-off time (in us x10)
#define REG_RES_IDC					214	// Actual DC current value (in A)
#define REG_RES_DIDT				215	// Actual dI/dt value (in A/us x10)
#define REG_RES_QRR_INT				216	// Reverse recovery charge (integration mode) (in uC x10)
#define REG_RES_VD					217	// Actual direct voltage value (in V)
//
#define REG_CANA_BUSOFF_COUNTER		220 // Counter of bus-off states
#define REG_CANA_STATUS_REG			221	// CAN status register (32 bit)
#define REG_CANA_STATUS_REG_32		222
#define REG_CANA_DIAG_TEC			223	// CAN TEC
#define REG_CANA_DIAG_REC			224	// CAN REC
//
#define REG_CANB_BUSOFF_COUNTER		225 // Counter of bus-off states
#define REG_CANB_STATUS_REG			226	// CAN status register (32 bit)
#define REG_CANB_STATUS_REG_32		227
#define REG_CANB_DIAG_TEC			228	// CAN TEC
#define REG_CANB_DIAG_REC			229	// CAN REC
//
#define REG_DIAG_OUT_1				230	// Output diagnostic register 1
#define REG_DIAG_OUT_2				231	// Output diagnostic register 2
#define REG_DIAG_OUT_3				232	// Output diagnostic register 3
//
#define REG_EP_ELEMENT_FRACT		235	// Elementary fraction length (in ns)
#define REG_EP_STEP_FRACT_CNT		236	// Number of elementary fractions in the EP single step
//
#define REG_PRESSURE				240	// Pressure flag
#define REG_CSU_VOLTAGE				241	// CSU voltage (V*10)
//
#define REG_SP__3					255
//
// ----------------------------------------
//
#define REG_FWINFO_SLAVE_NID		256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID		257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN			260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN		261	// Begining of the information string record
//
// DCU COMMANDS, REGISTER, STATES
//
#define REG_I_TO_V_INTPS_EXT_OFFSET	132 // Добавочное смещение преобразования тока в напряжение внутреннего источника
#define REG_I_TO_V_INTPS_EXT_K		133	// Добавочный коэффицент преобразования тока в напряжение внутреннего источника
#define REG_I_TO_V_INTPS_EXT_K2		134	// Добавочный квадратичный коэффициент преобразования тока в напряжение внутреннего источника
//
// RCU COMMANDS, REGISTER, STATES
//
#define REG_I_TO_V_INTPS_EXT_OFFSET	132	// Добавочное смещение преобразования тока в напряжение внутреннего источника
#define REG_I_TO_V_INTPS_EXT_K		133	// Добавочный коэффицент преобразования тока в напряжение внутреннего источника
#define REG_I_TO_V_INTPS_EXT_K2		134	// Добавочный квадратичный коэффициент преобразования тока в напряжение внутреннего источника
#define REG_CTRL1_EXT_OFFSET		135	// Добавочное смещение грубой подстройки тока при скрости спада
#define REG_CTRL1_EXT_K				136	// Добавочный коэффицент грубой подстройки тока при скрости спада
//
// CROVU SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_CROVU_APPLY_SETTINGS	10	// Apply settings to cells
#define ACT_CROVU_ENABLE_EXT_SYNC	106	// Enable external SYNC
//
// Registers
#define REG_CROVU_DESIRED_VOLTAGE	128	// Desired plate voltage
#define REG_CROVU_VOLTAGE_RATE		129	// dV/dt rate
//
// ----------------------------------------


// FCROVU SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_FCROVU_CONFIG			101	// Aply settings

// Registers
#define REG_FCROVU_V_RATE_VALUE		128	// dV/dt rate [V/us]
#define REG_FCROVU_I_SHORT_CIRCUIT	130	// Short circiut current (400, 800, 1200, 1600)А
//
// ----------------------------------------


// SCOPE SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_SCOPE_START_TEST		100	// Start test with defined parameters
#define ACT_SCOPE_STOP_TEST			101	// Force test to stop
//
// Registers
#define REG_SCOPE_CURRENT_AMPL		128	// Current amplitude (in A)
#define REG_SCOPE_MEASURE_MODE		129	// Select sampling mode (Qrr or Qrr-tq)
#define REG_SCOPE_TR_050_METHOD		130	// Use 50% level of Irr to detect tr time
//
// Results
#define REG_SCOPE_RESULT_IRR		201	// Reverse recovery current amplitude (in A)
#define REG_SCOPE_RESULT_TRR		202	// Reverse recovery time (in us x10)
#define REG_SCOPE_RESULT_QRR		203	// Reverse recovery charge (in uQ)
#define REG_SCOPE_RESULT_ZERO		204	// Zero-cross time (in us x10)
#define REG_SCOPE_RESULT_ZERO_V		205	// Zero-cross time for on-state voltage (in us x10)
#define REG_SCOPE_RESULT_DIDT		206	// Actual value of dIdt (in A/us x10)
#define REG_SCOPE_RESULT_IDC		207	// Actual value of Idc (in A)
#define REG_SCOPE_RESULT_VD			208	// Actual value of Vd (in V)
//
#define REG_SCOPE_EP_ELEMENT_FRACT	220	// Elementary fraction length (in ns)
#define REG_SCOPE_EP_STEP_FRACT_CNT	221	// Number of elementary fractions in the EP single step
//
// Problems
#define PROBLEM_SCOPE_NONE			0	// No problem
#define PROBLEM_SCOPE_CALC_IRR		1	// Problem calculating Irr
#define PROBLEM_SCOPE_CALC_IRR_025	2	// Problem calculating 25% fraction Irr
#define PROBLEM_SCOPE_CALC_IRR_090	3	// Problem calculating 90% fraction Irr
#define PROBLEM_SCOPE_CALC_VZ		4	// Problem calculating V zero crossing
#define PROBLEM_SCOPE_CALC_DIDT		5	// Problem calculating actual dIdt
#define PROBLEM_SAFETY				6	// Problem safety
//
// Endpoints
#define EP_SCOPE_IDC				1	// Scope current data
#define EP_SCOPE_VD					2	// Scope voltage data
// ----------------------------------------

// ENDPOINTS
//
#define EP_Current					1	// Current data
#define EP_Voltage					2	// Voltage data
//
#define EP_DIAG1_DevTrig			3	//
#define EP_DIAG2_OSVTime			4	//
#define EP_DIAG3_Irr				5	//
#define EP_DIAG4_Trr				6	//
#define EP_DIAG5_Qrr				7	//
#define EP_DIAG6_Idc				8	//
#define EP_DIAG7_ZeroI				9	//
#define EP_DIAG8_ZeroV				10	//
#define EP_DIAG9_dIdt				11	//
//
#define EP_SlaveData				12	// Data obtained from slave device

// OPRESULTS
//
#define OPRESULT_NONE				0	// No information or not finished
#define OPRESULT_OK					1	// Operation was successful
#define OPRESULT_FAIL				2	// Operation failed

// MEASUREMENT MODES
//
#define MODE_QRR_ONLY				0	// Measure only reverse recovery (Qrr) parameters
#define MODE_QRR_TQ					1	// Measure Qrr-tq parameters
#define MODE_DVDT_ONLY				2	// dVdt process only

// FAULT CODES
//
#define FAULT_NONE					0	// No fault
#define FAULT_PROTOCOL				1	// Error in communication protocol
#define FAULT_LOGIC_GENERAL			2	// High-level logic state-machine error
#define FAULT_LOGIC_CROVU			3	// CROVU node error
#define FAULT_LOGIC_FCROVU			4	// FCROVU node error
#define FAULT_LOGIC_DCU1			5	// DCU1 node error
#define FAULT_LOGIC_DCU2			6	// DCU2 node error
#define FAULT_LOGIC_DCU3			7	// DCU3 node error
#define FAULT_LOGIC_RCU1			8	// RCU1 node error
#define FAULT_LOGIC_RCU2			9	// RCU2 node error
#define FAULT_LOGIC_RCU3			10	// RCU3 node error
#define FAULT_LOGIC_SCOPE			11	// SCOPE node error
#define FAULT_PRESSURE				12	// No pressure

// FAULT EXTENDED CODES
//
#define FAULTEX_NONE				0	// No extended information
#define FAULTEX_PON_WRONG_STATE		1	// Wrong state during power-on stage
#define FAULTEX_PON_TIMEOUT			2	// Timeout during power-on stage
#define FAULTEX_CFG_WRONG_STATE		3	// Wrong state during configuration stage
#define FAULTEX_CFG_TIMEOUT			4	// Timeout during configuration stage
#define FAULTEX_POFF_WRONG_STATE	5	// Wrong state during power-off stage
#define FAULTEX_POFF_TIMEOUT		6	// Timeout during power-off stage
#define FAULTEX_READ_WRONG_STATE	7	// Wrong state during read data
#define FAULTEX_READ_TIMEOUT		8	// Timeout during read data
#define FAULTEX_PRECFG_TIMEOUT		9	// Timeout during preconfig stage

// WARNING CODES
//
#define WARNING_NONE				0
#define WARNING_MANUAL_STOP			1	// Requested manual stop of the measurement
#define WARNING_NO_DIRECT_CURRENT	2	// No direct current
#define WARNING_SCOPE_CALC_FAILED	3	// Calculations on scope node are failed
#define WARNING_IRR_TO_HIGH			4	// Reverse current is too high
#define WARNING_DEVICE_TRIGGERED	5	// Device remains in trigged state
#define WARNING_WATCHDOG_RESET		1001	// System has been reseted by WD

// DISABLE CODES
//
#define DISABLE_NONE				0
#define DISABLE_BAD_CLOCK			1001	// Problem with main oscillator

// USER ERROR CODES
//
#define ERR_NONE					0	// No error
#define ERR_CONFIGURATION_LOCKED	1	// Device is locked for writing
#define ERR_OPERATION_BLOCKED		2	// Operation can't be done due to current device state
#define ERR_DEVICE_NOT_READY		3	// Device isn't ready to switch state
#define ERR_WRONG_PWD				4	// Wrong password - unlock failed


#endif // __DEV_OBJ_DIC_H
