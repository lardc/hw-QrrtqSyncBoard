// -----------------------------------------
// Constraints for tunable parameters
// ----------------------------------------

// Header
#include "Constraints.h"
#include "DeviceObjectDictionary.h"

#define NO		0	// equal to FALSE
#define YES		1	// equal to TRUE

// Constants
//
const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE] =
                                       {
                                    		   {NO, YES, NO},															// 0
                                    		   {NO, YES, NO},															// 1
                                    		   {NO, YES, NO},															// 2
                                    		   {NO, YES, NO},															// 3
                                    		   {NO, YES, NO},															// 4
											   {NO, YES, NO},															// 5
											   {NO, YES, NO},															// 6
											   {NO, YES, NO},															// 7
											   {NO, YES, NO},															// 8
											   {NO, YES, NO},															// 9
											   {0, INT16U_MAX, 0},														// 10
											   {0, INT16U_MAX, 0},														// 11
											   {0, INT16U_MAX, 0},														// 12
											   {0, INT16U_MAX, 0},														// 13
											   {0, INT16U_MAX, 0},														// 14
											   {0, INT16U_MAX, 0},														// 15
											   {0, INT16U_MAX, 0},														// 16
											   {0, INT16U_MAX, 0},														// 17
											   {NO, YES, NO},															// 18
											   {0, 50, 0},																// 19
											   {0, INT16U_MAX, 0},														// 20
											   {0, INT16U_MAX, 0},														// 21
											   {0, DRCU_CURRENT_MAX, 0},												// 22
											   {0, DRCU_CURRENT_MAX, 0},												// 23
											   {0, 0, 0},																// 24
											   {0, DCU_I_RISE_RATE_MAX, DCU_I_RISE_RATE_DEF},							// 25
											   {0, MAX_GATE_DRV_TURN_ON_DELAY, 0},										// 26
											   {0, INT16U_MAX, 0},														// 27
											   {0, MAX_UNIT_DRCU, DEF_UNIT_DRCU},										// 28
											   {0, 0, 0},																// 29
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 30
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 31
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 32
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 33
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 34
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 35
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 36
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 37
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 38
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 39
											   {0, MAX_RCU_TRIG_OFFSET, 0},												// 40
											   {0, DIRECT_CURRENT_MAX, 0},												// 41
											   {0, MAX_FCROVU_TRIG_OFFSET, 0},											// 42
											   {0, MAX_FCROVU_TRIG_OFFSET, 0},											// 43
											   {0, MAX_FCROVU_TRIG_OFFSET, 0},											// 44
											   {0, MAX_FCROVU_TRIG_OFFSET, 0},											// 45
											   {0, 0, 0},																// 46
											   {0, 0, 0},																// 47
											   {0, 0, 0},																// 48
											   {0, 0, 0},																// 49
											   {0, MAX_FALL_RATE, FALL_RATE_R0_DEF},									// 50
											   {0, MAX_FALL_RATE, FALL_RATE_R1_DEF},									// 51
											   {0, MAX_FALL_RATE, FALL_RATE_R2_DEF},									// 52
											   {0, MAX_FALL_RATE, FALL_RATE_R3_DEF},									// 53
											   {0, MAX_FALL_RATE, FALL_RATE_R4_DEF},									// 54
											   {0, MAX_FALL_RATE, FALL_RATE_R5_DEF},									// 55
											   {0, MAX_FALL_RATE, FALL_RATE_R6_DEF},									// 56
											   {0, MAX_FALL_RATE, FALL_RATE_R7_DEF},									// 57
											   {0, MAX_FALL_RATE, FALL_RATE_R8_DEF},									// 58
											   {0, MAX_FALL_RATE, FALL_RATE_R9_DEF},									// 59
											   {0, MAX_FALL_RATE, FALL_RATE_R10_DEF},									// 60
											   {0, 0, 0},																// 61
											   {0, 0, 0},																// 62
											   {0, 0, 0},																// 63
											   {0, 0, 0},																// 64
											   {0, 0, 0},																// 65
											   {0, 0, 0},																// 66
											   {0, 0, 0},																// 67
											   {0, 0, 0},																// 68
											   {0, 0, 0},																// 69
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 70
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 71
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 72
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 73
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 74
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 75
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 76
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 77
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 78
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 79
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 80
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 81
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 82
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 83
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 84
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 85
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 86
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 87
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 88
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 89
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 91
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 92
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 93
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 94
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 95
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 96
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 97
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 98
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 99
											   {0, INT16U_MAX, I_TO_V_OFFSET_DEF},										// 100
											   {0, INT16U_MAX, I_TO_V_K_DEF},											// 101
											   {0, INT16U_MAX, I_TO_V_K2_DEF},											// 102
                                    		   {0, INT16U_MAX, CTRL_OFFSET_DEF},										// 103
                                    		   {0, INT16U_MAX, CTRL_K_DEF},												// 104
                                    		   {0, INT16U_MAX, CTRL_OFFSET_DEF},										// 105
                                    		   {0, INT16U_MAX, CTRL_K_DEF},												// 106
                                    		   {0, INT16U_MAX, I_TO_DAC_P0_DEF},										// 107
                                    		   {0, INT16U_MAX, I_TO_DAC_P1_DEF},										// 108
                                    		   {0, INT16U_MAX, I_TO_DAC_P2_DEF},										// 109
                                    		   {0, 0, 0},																// 110
                                    		   {0, 0, 0},																// 111
                                    		   {0, 0, 0},																// 112
                                    		   {0, 0, 0},																// 113
                                    		   {0, 0, 0},																// 114
                                    		   {0, 0, 0},																// 115
                                    		   {0, 0, 0},																// 116
                                    		   {0, 0, 0},																// 117
                                    		   {0, 0, 0},																// 118
                                    		   {0, 0, 0},																// 119
                                    		   {0, 0, 0},																// 120
                                    		   {0, 0, 0},																// 121
                                    		   {0, 0, 0},																// 122
                                    		   {0, 0, 0},																// 123
                                    		   {0, 0, 0},																// 124
                                    		   {0, 0, 0},																// 125
                                    		   {0, 0, 0},																// 126
                                    		   {0, 0, 0}																// 127
                                       };

const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START] =
                                      {
                                    		   {MODE_QRR_ONLY, MODE_DVDT_ONLY, MODE_QRR_ONLY},							// 128
											   {DIRECT_CURRENT_MIN, DIRECT_CURRENT_MAX, DIRECT_CURRENT_DEF},			// 129
											   {IDC_PULSE_WIDTH_MIN, IDC_PULSE_WIDTH_MAX, IDC_PULSE_WIDTH_DEF},			// 130
											   {IRC_SYNC_WIDTH_MIN, IRC_SYNC_WIDTH_MAX, IRC_SYNC_WIDTH_DEF},			// 131
											   {REVERSE_RATE_MIN, REVERSE_RATE_MAX, REVERSE_RATE_DEF},					// 132
											   {OS_VOLATGE_MIN, OS_VOLATGE_MAX, OS_VOLATGE_DEF},						// 133
											   {OSV_RATE_MIN, OSV_RATE_MAX, OSV_RATE_DEF},								// 134
											   {TQ_SINGLE_PULSE_MIN, TQ_SINGLE_PULSE_MAX, TQ_SINGLE_PULSE_DEF},			// 135
											   {NO, YES, NO},															// 136
											   {OSV_CURRENT_MIN, OSV_CURRENT_MAX, OSV_CURRENT_DEF},						// 137
											   {CSU_VOLTAGE_MIN, CSU_VOLTAGE_MAX, CSU_VOLTAGE_DEF},						// 138
											   {CSU_VOLTAGE_HYST_MIN, CSU_VOLTAGE_HYST_MAX, CSU_VOLTAGE_HYST_DEF},		// 139
                                    		   {NO, YES, NO},															// 140
                                    		   {0, 0, 0},																// 141
                                    		   {0, 0, 0},																// 142
                                    		   {0, 0, 0},																// 143
                                    		   {0, 0, 0},																// 144
                                    		   {0, 0, 0},																// 145
                                    		   {0, 0, 0},																// 146
                                    		   {0, 0, 0},																// 147
											   {0, 0, 0},																// 148
											   {0, 0, 0},																// 149
											   {0, INT16U_MAX, 0},														// 150
											   {0, INT16U_MAX, 0},														// 151
											   {0, INT16U_MAX, 0},														// 152
											   {NO, YES, NO},															// 153
											   {0, 0, 0},																// 154
											   {0, 0, 0},																// 155
											   {0, 0, 0},																// 156
											   {0, 0, 0},																// 157
											   {0, 0, 0},																// 158
											   {0, 0, 0},																// 159
											   {0, INT16U_MAX, 0},														// 160
											   {0, INT16U_MAX, 0},														// 161
											   {0, INT16U_MAX, 0},														// 162
											   {0, 0, 0},																// 163
											   {0, 0, 0},																// 164
											   {0, 0, 0},																// 165
											   {0, 0, 0},																// 166
											   {0, 0, 0},																// 167
											   {0, 0, 0},																// 168
											   {0, 0, 0},																// 169
											   {0, INT16U_MAX, 0},														// 170
											   {0, INT16U_MAX, 0},														// 171
											   {0, INT16U_MAX, 0},														// 172
											   {0, INT16U_MAX, 0},														// 173
											   {0, 0, 0},																// 174
											   {0, 0, 0},																// 175
											   {0, 0, 0},																// 176
											   {0, 0, 0},																// 177
											   {0, 0, 0},																// 178
											   {0, 0, 0},																// 179
											   {0, 0, 0},																// 180
											   {0, 0, 0},																// 181
											   {0, 0, 0},																// 182
											   {0, 0, 0},																// 183
											   {0, 0, 0},																// 184
											   {0, 0, 0},																// 185
											   {0, 0, 0},																// 186
											   {0, 0, 0},																// 187
											   {0, 0, 0},																// 188
											   {0, 0, 0},																// 189
											   {0, 0, 0},																// 190
											   {INT16U_MAX, 0, 0}														// 191
                                      };

// No more
