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
#define REG_EMULATE_FCROVU			0	// Enable FCROVU emulation
#define REG_EMULATE_CROVU			1	// Enable CROVU emulation
#define REG_EMULATE_DCU				2	// Enable DCU emulation
#define REG_EMULATE_RCU				3	// Enable DCU emulation
#define REG_EMULATE_SCOPE			4	// Enable SCOPE emulation
#define REG_EMULATE_CSU				5	// Enable CSU emulation
//
#define REG_DCU1_CAN_ADDR			6	// DCU1 CAN address
#define REG_DCU2_CAN_ADDR			7	// DCU2 CAN address
#define REG_DCU3_CAN_ADDR			8	// DCU3 CAN address
#define REG_RCU1_CAN_ADDR			9	// RCU1 CAN address
#define REG_RCU2_CAN_ADDR			10	// RCU2 CAN address
#define REG_RCU3_CAN_ADDR			11	// RCU3 CAN address
//
#define REG_DCU1_ACTIVE				12	// DCU1 switching in active/deactive state
#define REG_DCU2_ACTIVE				13	// DCU2 switching in active/deactive state
#define REG_DCU3_ACTIVE				14	// DCU3 switching in active/deactive state
#define REG_RCU1_ACTIVE				15	// RCU1 switching in active/deactive state
#define REG_RCU2_ACTIVE				16	// RCU2 switching in active/deactive state
#define REG_RCU3_ACTIVE				17	// RCU3 switching in active/deactive state
//
#define REG_CSU_V_C					18 	// CSU voltage coefficient
//
#define REG_SAFETY_EN				20	// Safety enable
// ----------------------------------------
//
#define REG_MODE					128	// Measurement mode
#define REG_DIRECT_CURRENT			129	// Direct current amplitude (in A)
#define REG_DCU_PULSE_WIDTH			130	// Direct current pulse duration (in us)
#define REG_DCU_RISE_RATE			131	// Direct current rise rate (in A/us x10)
#define REG_DCU_FALL_RATE			132 // Direct current fall rate (in A/us x10)
#define REG_OFF_STATE_VOLTAGE		133 // Off-state voltage (in V)
#define REG_OSV_RATE				134 // Off-state voltage rise rate (in V/us)
#define REG_TRIG_TIME				135	// Time to apply on-state voltage in single-pulse mode (in us)
#define REG_TRR_DETECTION_MODE		136	// Select trr detection mode (0 - 90%-25%; 1 - 90%-50%)
#define REG_FCROVU_I_SHORT			137
//
#define REG_DIAG_NID				150	// Node-id diagnostic register
#define REG_DIAG_IN_1				151	// Input diagnostic register 1
#define REG_DIAG_IN_2				152	// Input diagnostic register 2
#define REG_DIAG_DISABLE_PLOT_READ	153	// Disable plot read from scope
//
#define REG_DCU_SYNC_DELAY			160
#define REG_RCU_SYNC_DELAY			161
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
#define REG_DC_READY_RETRIES		205	// Number of attempts to start direct current
//
#define REG_RES_QRR					210	// Reverse recovery charge (in uC x10)
#define REG_RES_IRR					211	// Reverse recovery current amplitude (in A)
#define REG_RES_TRR					212	// Reverse recovery time (in us x10)
#define REG_RES_TQ					213	// Turn-off time (in us x10)
#define REG_RES_IDC					214	// Actual DC current value (in A)
#define REG_RES_DIDT				215	// Actual dI/dt value (in A/us x10)
#define REG_RES_QRR_INT				216	// Reverse recovery charge (integration mode) (in uC x10)
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
#define REG_SP__3					255
//
// ----------------------------------------

// CROVU SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_CROVU_ENABLE_POWER		1	// Enable flyback converter
#define ACT_CROVU_DISABLE_POWER		2	// Disable flyback converter
#define ACT_CROVU_CLR_FAULT			3	// Clear fault (try switch state from FAULT to NONE)
#define ACT_CROVU_CLR_WARNING		4	// Clear warning
//
#define ACT_CROVU_APPLY_SETTINGS	101	// Apply settings to cells
//
#define ACT_CROVU_ENABLE_EXT_SYNC	106	// Enable external SYNC
//
// Registers
#define REG_CROVU_DESIRED_VOLTAGE	128	// Desired plate voltage
#define REG_CROVU_VOLTAGE_RATE		129	// dV/dt rate
//
#define REG_CROVU_DEV_STATE			192	// Device state
#define REG_CROVU_FAULT_REASON		193	// Fault reason in the case DeviceState -> FAULT
#define REG_CROVU_DISABLE_REASON	194	// Disbale reason in the case DeviceState -> DISABLE
#define REG_CROVU_WARNING			195	// Warning if present
#define REG_CROVU_PROBLEM			196	// Problem if present
//
#define REG_CROVU_TEST_RESULT		198	// Test result
#define REG_CROVU_FAULT_REASON_EX	199 // External fault code
//
#define REG_CROVU_VOLTAGE_OK		200	// Charged summary
#define REG_CROVU_VOLTAGE_OK_1		201	// Charged flag 1
#define REG_CROVU_VOLTAGE_OK_2		202	// Charged flag 2
#define REG_CROVU_VOLTAGE_OK_3		203	// Charged flag 3
#define REG_CROVU_VOLTAGE_OK_4		204	// Charged flag 4
#define REG_CROVU_VOLTAGE_OK_5		205	// Charged flag 5
#define REG_CROVU_VOLTAGE_OK_6		206	// Charged flag 6
#define REG_CROVU_ACTUAL_VOLTAGE_1	207	// Actual capacitor level 1
#define REG_CROVU_ACTUAL_VOLTAGE_2	208	// Actual capacitor level 2
#define REG_CROVU_ACTUAL_VOLTAGE_3	209	// Actual capacitor level 3
#define REG_CROVU_ACTUAL_VOLTAGE_4	210	// Actual capacitor level 4
#define REG_CROVU_ACTUAL_VOLTAGE_5	211	// Actual capacitor level 5
#define REG_CROVU_ACTUAL_VOLTAGE_6	212	// Actual capacitor level 6
#define REG_CROVU_CELL_STATE_1		213	// Cell state 1
#define REG_CROVU_CELL_STATE_2		214	// Cell state 2
#define REG_CROVU_CELL_STATE_3		215	// Cell state 3
#define REG_CROVU_CELL_STATE_4		216	// Cell state 4
#define REG_CROVU_CELL_STATE_5		217	// Cell state 5
#define REG_CROVU_CELL_STATE_6		218	// Cell state 6
//
// States
#define DS_CROVU_NONE				0
#define DS_CROVU_FAULT				1
#define DS_CROVU_DISABLED			2
#define DS_CROVU_READY				3
#define DS_CROVU_IN_PROCESS			4
#define DS_CROVU_EXT_SYNC			5
//
// ----------------------------------------


// FCROVU SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_FCROVU_ENABLE_POWER					1	// Power enable
#define ACT_FCROVU_DISABLE_POWER				2	// Power disable
#define ACT_FCROVU_FAULT_CLEAR					3	// Clear fault
#define ACT_FCROVU_WARNING_CLEAR				4	// Clear warning
//
#define ACT_FCROVU_START_PULSE					100	// Start pulse
#define ACT_FCROVU_CONFIG						101	// Aply settings
//

// Registers
#define REG_FCROVU_V_RATE_VALUE					128	// dV/dt rate [V/us]
#define REG_FCROVU_I_SHORT_CIRCUIT				130	// Short circiut current (400, 800, 1200, 1600)À
//
#define REG_FCROVU_DEV_STATE					192	// Device state
#define REG_FCROVU_FAULT_REASON					193	// Fault reason
#define REG_FCROVU_DISABLE_REASON				194	// Disable reason
#define REG_FCROVU_WARNING						195	// Warning
#define REG_FCROVU_PROBLEM						196	// Problem
#define REG_FCROVU_TEST_FINISHED				197
//
#define REG_FCROVU_DBG_VSO_VALUE				200	// Battery volatge (V)
//

// States
#define DS_FCROVU_NONE							0
#define DS_FCROVU_FAULT							1
#define DS_FCROVU_DISABLED						2
#define DS_FCROVU_CHARGING						3
#define DS_FCROVU_READY							4
#define DS_FCROVU_CONFIG_READY					5
#define DS_FCROVU_IN_PROCESS					6
//
// ----------------------------------------


// DCU SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_DCU_ENABLE_POWER					1	// Power enable
#define ACT_DCU_DISABLE_POWER					2	// Power disable
#define ACT_DCU_FAULT_CLEAR						3	// Clear fault
#define ACT_DCU_WARNING_CLEAR					4	// Clear warning
//
#define ACT_DCU_START_PULSE						100	// Start pulse
#define ACT_DCU_CONFIG							101	// Aplly settings
//
//
// Registers
#define REG_DCU_I_MAX_VALUE						129	// Direct current value A
#define REG_DCU_I_RATE_RISE						130	// Direct current rise rate A/us
#define REG_DCU_I_RATE_FALL						131	// Direct current fall rate A/us
//
#define REG_DCU_DEV_STATE						192	// Device state
#define REG_DCU_FAULT_REASON					193	// Fault reason
#define REG_DCU_DISABLE_REASON					194	// Disable reason
#define REG_DCU_WARNING							195	// Warning if present
#define REG_DCU_PROBLEM							196	// Problem if present
#define REG_DCU_TEST_FINISHED					197 // Test result
//
#define REG_DCU_DBG_VSO_VALUE					200	// Bateery voltage V
#define REG_DCU_DBG_I_AVG_DUT_VALUE				201	// Direct average current A
#define REG_DCU_DBG_I_MAX_DUT_VALUE				202	// Direct
//

// States
#define DS_DCU_NONE								0
#define DS_DCU_FAULT							1
#define DS_DCU_DISABLED							2
#define DS_DCU_CHARGING							3
#define DS_DCU_READY							4
#define DS_DCU_CONFIG_READY						5
#define DS_DCU_IN_PROCESS						6
//
// ----------------------------------------


// RCU SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_RCU_ENABLE_POWER					1	// Power enable
#define ACT_RCU_DISABLE_POWER					2	// Power disable
#define ACT_RCU_FAULT_CLEAR						3	// Clear fault
#define ACT_RCU_WARNING_CLEAR					4	// Clear warning
//
#define ACT_RCU_START_PULSE						100	// Start pulse
#define ACT_RCU_CONFIG							101	// Aplly settings
//
//
// Registers
#define REG_RCU_I_MAX_VALUE						129	// Direct current value A
#define REG_RCU_I_RATE_RISE						130	// Direct current rise rate A/us
#define REG_RCU_I_RATE_FALL						131	// Direct current fall rate A/us
//
#define REG_RCU_DEV_STATE						192	// Device state
#define REG_RCU_FAULT_REASON					193	// Fault reason
#define REG_RCU_DISABLE_REASON					194	// Disable reason
#define REG_RCU_WARNING							195	// Warning if present
#define REG_RCU_PROBLEM							196	// Problem if present
#define REG_RCU_TEST_FINISHED					197 // Test result
//
#define REG_RCU_DBG_VSO_VALUE					200	// Bateery voltage V
#define REG_RCU_DBG_I_AVG_DUT_VALUE				201	// Direct average current A
#define REG_RCU_DBG_I_MAX_DUT_VALUE				202	// Direct
//

// States
#define DS_RCU_NONE								0
#define DS_RCU_FAULT							1
#define DS_RCU_DISABLED							2
#define DS_RCU_CHARGING							3
#define DS_RCU_READY							4
#define DS_RCU_CONFIG_READY						5
#define DS_RCU_IN_PROCESS						6
//
// ----------------------------------------

// CSU STATES
//
// States
#define DS_CSU_NONE								0
#define DS_CSU_FAULT							1
#define DS_CSU_DISABLED							2
#define DS_CSU_CHARGING							3
#define DS_CSU_DISCHARGING						3
#define DS_CSU_READY							4
//
// ----------------------------------------

// SCOPE SLAVE COMMANDS, REGISTERS, STATES
//
// Commands
#define ACT_SCOPE_CLR_FAULT			3	// Clear fault (try switch state from FAULT to NONE)
#define ACT_SCOPE_CLR_WARNING		4	// Clear warning
//
#define ACT_SCOPE_DIAG_GEN_READ_EP	10	// Fill diagnostic endpoint with values
//
#define ACT_SCOPE_START_TEST		100	// Start test with defined parameters
#define ACT_SCOPE_STOP_TEST			101	// Force test to stop
//
// Registers
#define REG_SCOPE_CURRENT_AMPL		128	// Current amplitude (in A)
#define REG_SCOPE_MEASURE_MODE		129	// Select sampling mode (Qrr or Qrr-tq)
#define REG_SCOPE_TR_050_METHOD		130	// Use 50% level of Irr to detect tr time
//
#define REG_SCOPE_DEV_STATE			192	// Device state
#define REG_SCOPE_FAULT_REASON		193	// Fault reason in the case DeviceState -> FAULT
#define REG_SCOPE_DISABLE_REASON	194	// Disbale reason in the case DeviceState -> DISABLE
#define REG_SCOPE_WARNING			195	// Warning if present
#define REG_SCOPE_PROBLEM			196	// Problem if present
#define REG_SCOPE_DF_REASON_EX		197	// Fault or disable extended reason
//
// Results
#define REG_SCOPE_FINISHED			200	// Operation result
#define REG_SCOPE_RESULT_IRR		201	// Reverse recovery current amplitude (in A)
#define REG_SCOPE_RESULT_TRR		202	// Reverse recovery time (in us x10)
#define REG_SCOPE_RESULT_QRR		203	// Reverse recovery charge (in uQ)
#define REG_SCOPE_RESULT_ZERO		204	// Zero-cross time (in us x10)
#define REG_SCOPE_RESULT_ZERO_V		205	// Zero-cross time for on-state voltage (in us x10)
#define REG_SCOPE_RESULT_DIDT		206	// Actual value of dIdt (in A/us x10)
//
// States
#define DS_SCOPE_NONE				0
#define DS_SCOPE_FAULT				1
#define DS_SCOPE_DISABLED			2
#define DS_SCOPE_IN_PROCESS			3
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


// WARNING CODES
//
#define WARNING_NONE				0
#define WARNING_MANUAL_STOP			1	// Requested manual stop of the measurement
#define WARNING_NO_DIRECT_CURRENT	2	// No direct current
#define WARNING_SCOPE_CALC_FAILED	3	// Calculations on scope node are failed
#define WARNING_IRR_TO_HIGH			4	// Reverse current is too high
#define WARNING_DEVICE_TRIGGERED	5	// Device remains in trigged state
#define WARNING_DC_SET_TO_HIGH		6	// Idc set is to high
#define WARNING_RC_SET_TO_HIGH		7	// Irc set is to high
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
