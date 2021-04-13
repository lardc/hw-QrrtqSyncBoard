#ifndef __COMMON_DICTIONARY_H
#define __COMMON_DICTIONARY_H

// Actions
#define COMM_ACT_ENABLE_POWER				1	// ��������� �������
#define COMM_ACT_DISABLE_POWER				2	// ���������� �������
#define COMM_ACT_FAULT_CLEAR				3	// ������� fault
#define COMM_ACT_WARNING_CLEAR				4	// ������� warning

// Registers
#define COMM_REG_DEV_STATE					192	// ������� ���������
#define COMM_REG_FAULT_REASON				193	// ������� Fault
#define COMM_REG_DISABLE_REASON				194	// ������� Disable
#define COMM_REG_WARNING					195	// ������� Warning
#define COMM_REG_PROBLEM					196	// ������� Problem
#define COMM_REG_OP_RESULT					197	// ������� ���������� ��������

// Operation results
#define COMM_OPRESULT_NONE					0	// No information or not finished
#define COMM_OPRESULT_OK					1	// Operation was successful
#define COMM_OPRESULT_FAIL					2	// Operation failed

//  User Errors
#define COMM_ERR_NONE						0
#define COMM_ERR_CONFIGURATION_LOCKED		1	//  ���������� �������� �� ������
#define COMM_ERR_OPERATION_BLOCKED			2	//  �������� �� ����� ���� ��������� � ������� ��������� ����������
#define COMM_ERR_DEVICE_NOT_READY			3	//  ���������� �� ������ ��� ����� ���������
#define COMM_ERR_WRONG_PWD					4	//  ������������ ����
#define COMM_ERR_BAD_CONFIG					5	//  ������� ������� ��������� � �������� �������������

// States
typedef enum __CommonDeviceState
{
	CDS_None = 0,
	CDS_Fault = 1,
	CDS_Disabled = 2,
	CDS_Ready = 3,
	CDS_InProcess = 4
} CommonDeviceState;

#endif // __COMMON_DICTIONARY_H
