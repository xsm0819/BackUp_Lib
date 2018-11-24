#include "Terminal.h"
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "string.h"
#include "CallBoard.h"
#include "GSM.h"

char TerminalBufer[100];
unsigned char count = 0;
unsigned char Terminal_CMD = 0;
unsigned char GsmTermSession = 0, CallTermSession = 0;

char PASSWORD[8]="PASSWORD";

void InitUART_Terminal	(unsigned long BRR)
{
	 /* Enable USART1 and GPIOA clock */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	    /* NVIC Configuration */
	    NVIC_InitTypeDef NVIC_InitStructure;
	    /* Enable the USARTx Interrupt */
	    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	    NVIC_Init(&NVIC_InitStructure);

	    USART_InitTypeDef USART_InitStructure;

	    USART_InitStructure.USART_BaudRate = BRR;
	    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	    USART_InitStructure.USART_StopBits = USART_StopBits_1;
	    USART_InitStructure.USART_Parity = USART_Parity_No;
	    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	    USART_Init(USART1, &USART_InitStructure);
	    USART_Cmd(USART1, ENABLE);

	    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void Terminal_Send(char chr)
{
  while(!(USART1->SR & USART_SR_TC));
  USART1->DR = chr;
}

void Terminal_Send_String(char* str)
{
  int i=0;
  while(str[i])
	  Terminal_Send(str[i++]);
}


unsigned char LevelAccess = 0;

		// Обработчик всех прерываний от USART1
void USART1_IRQHandler(void)
{
	// Выясняем, какое именно событие вызвало прерывание. Если это приём байта в RxD - обрабатываем.
	if (USART1->SR & USART_SR_RXNE)
		{
		USART1->SR&=~USART_SR_RXNE;
		TerminalBufer[count] = USART1->DR;

		if (TerminalBufer[count]=='\n')
		{
			if (CallTermSession)
			{
				CAN_SendString(TerminalBufer);
			}
			if (GsmTermSession)
			{
				GSM_Send_String(TerminalBufer);
			}
			Search_Command();
		}
		else
			count++;
		}
}

void Search_Command (void)
{
		if ((strstr(TerminalBufer, CMD_StartSession))!=0)
		{
			LevelAccess = 1;
			Terminal_Send_String("Start Session\n");
		}
		else if ((strstr(TerminalBufer, CMD_EndSession))!=0)
		{
			LevelAccess = 0;
			Terminal_Send_String("End Session\n");
		}
		else if ((LevelAccess > 0)&&(LevelAccess < 3))
		{
			if ((strstr(TerminalBufer, CMD_ADMIN))!=0)
			{
				Terminal_Send_String("Please enter a password\n");
				LevelAccess = 2;
			}
			else if ((strstr(TerminalBufer, CMD_USER))!=0)
			{
				Terminal_Send_String("Welcome, USER\n");
				LevelAccess = 3;
			}
			else if (LevelAccess == 2)
			{
				if ((strstr(TerminalBufer, PASSWORD))!=0)
				{
					Terminal_Send_String("Welcome, Admin\n");
					LevelAccess = 4;
				}
				else Terminal_Send_String("PASSWORD NOT CORRECT\n");
			}
			else Terminal_Send_String("Command NOT CORRECT\n");
		}
		else if (LevelAccess > 2)
		{
		   if ((strstr(TerminalBufer, CMD_START_GSM_COMMAND))!=0)
			{
				GsmTermSession = 1;
				Terminal_Send_String("Start GSM Command Session\n");
			}
			else if ((strstr(TerminalBufer, CMD_STOP_GSM_COMMAND))!=0)
			{
				GsmTermSession = 0;
				Terminal_Send_String("Stop GSM Command Session\n");
			}
			else if ((strstr(TerminalBufer, CMD_START_CALL_COMMAND))!=0)
			{
				CallTermSession = 1;
				InitCAN(CAN_BRR_L);
				Terminal_Send_String("Start CALL Command Session\n");
			}
			else if ((strstr(TerminalBufer, CMD_STOP_CALL_COMMAND))!=0)
			{
				CallTermSession = 0;
				Terminal_Send_String("Stop CALL Command Session\n");
			}
		}
		else Terminal_Send_String("Command NOT CORRECT\n");
		Clear_Buffer();

	count = 0;
}

void Clear_Buffer (void)
{
	for (count = 0; count < 100; count++)
		TerminalBufer[count] = 0;
}
///////////////////////////////////////////////////////////

