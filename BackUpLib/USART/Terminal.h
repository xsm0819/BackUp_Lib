///////////////////////////////////////////////////
//	File:	Terminal.h
//	Autor:	XSM
//	Date:	07.08.2018
///////////////////////////////////////////////////
#ifndef _TERMINAL_H
#define _TERMINAL_H

#define TERMINAL_BRR	9600

extern char TerminalBufer[100];

extern unsigned char GsmTermSession, CallTermSession;
extern char PASSWORD[8];

#define CMD_StartSession		"START"
#define CMD_EndSession			"END"
#define CMD_ADMIN				"ADMIN"
#define CMD_USER				"USER"
#define CMD_STOP_GSM_COMMAND	"GSM_CMD_DISABLE"
#define CMD_START_GSM_COMMAND	"GSM_CMD_ENABLE"

#define CMD_STOP_CALL_COMMAND	"CALL_CMD_DISABLE"
#define CMD_START_CALL_COMMAND	"CALL_CMD_ENABLE"

#define CMD_RESET				"RESET_ODS"
#define CMD_CLEAR_CONFIG		"RESET_SETTINGS"

extern unsigned char Terminal_CMD;

void InitUART_Terminal	(unsigned long BRR);
void Terminal_Send(char chr);
void Terminal_Send_String(char* str);
void Search_Command (void);
void Clear_Buffer (void);

#endif /*	_TERMINAL_H	*/
