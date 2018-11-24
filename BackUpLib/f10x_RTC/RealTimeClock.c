
#include "RealTimeClock.h"
#include "strings.h"
#include "string.h"


#include "stm32f10x.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define nulyear 2001
#define nulmous 1
#define nulday 	1
#define nulh 	00
#define nulm 	00
#define nuls 	00

char TmpStr[50];

static int wys[12]={31,29,31,30,31,30,31,31,30,31,30,31};
static int Nwys[12]={31,28,31,30,31,30,31,31,30,31,30,31};

static unsigned int year=2016;
static unsigned char one=1;


// Local Variables
static unsigned char RTC_Seconds;
static unsigned char RTC_Minutes;
static unsigned char RTC_Hours;
static unsigned char RTC_Day;
static unsigned char RTC_Month;
static unsigned int  RTC_Year;



void RTC_IRQ(void)
{
	RTC->CRL &= ~0x0F;
	RTC_Processing();
}


void RTC_Init(void)
{
	 if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)        // Перевірка роботи годинника, якщо не увімкнені, то ініціалізувати
	   {
	    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  // Увімкнути тактування PWR та Backup
	    PWR->CR |= PWR_CR_DBP;                                  // Дозволити доступ до Backup області
	    RCC->BDCR |= RCC_BDCR_BDRST;                            // Скинути Backup область
	    RCC->BDCR &= ~RCC_BDCR_BDRST;
	    RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;      // Обрати LSE джерело (кварц 32768) і подати тактування
	    RCC->BDCR |= RCC_BDCR_LSEON;                            // Увімкнути LSE
	    while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){}// Дочекатись увімкнення
	    BKP->RTCCR |= 0;                                        // Калібрування RTC значення від 0 до 0x7F
	    while (!(RTC->CRL & RTC_CRL_RTOFF));                    // Перевірити чи закінчились зміни регістрів RTC
	    RTC->CRL  |=  RTC_CRL_CNF;                              // Дозволити запис до RTC
	    RTC->PRLL  = 0x7FFF;                                    // Налаштувати дільник на 32768 (32767+1)
	    RTC->CRL  &=  ~RTC_CRL_CNF;                             // Заборонити запис до RTC
	    while (!(RTC->CRL & RTC_CRL_RTOFF));                    // Дочекатись кінця запису
	    RTC->CRL &= (uint16_t)~RTC_CRL_RSF;                     // Синхронизувати RTC
	    while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF){}        // Дочекатись синхронізації
	    PWR->CR &= ~PWR_CR_DBP;                                 // Заборонити доступ до Backup області
	   }
}

uint32_t FtimeToCounter(ftime_t * ftime)
{
  uint8_t a;
  uint16_t y;
  uint8_t m;
  uint32_t JDN;

  a=(14-ftime->month)/12;
  y=ftime->year+4800-a;
  m=ftime->month+(12*a)-3;

  JDN=ftime->day;
  JDN+=(153*m+2)/5;
  JDN+=365*y;
  JDN+=y/4;
  JDN+=-y/100;
  JDN+=y/400;
  JDN+=-32045;
  JDN+=-JD0;    // приберемо дні які минули до 1 січня 2001 року
  JDN*=86400;        // перетворимо дні в секунди
  JDN+=(ftime->hour*3600);  // і додамо секунди поточного дня
  JDN+=(ftime->minute*60);
  JDN+=(ftime->second);

  return JDN;
}

ftime_t Time;

void RTC_Setup(unsigned char sec,	unsigned char min, unsigned char hours, unsigned char day, unsigned char mouh, unsigned int year)
{

Time.day = day;
Time.hour = hours;
Time.minute = min;
Time.month = mouh;
Time.second = sec;
Time.year = year;


	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;	// Включить тактирование PWR и Backup
	PWR->CR |= PWR_CR_DBP;                                  // Разрешить доступ к Backup области
	while (!(RTC->CRL & RTC_CRL_RTOFF));                    // Проверить закончены ли изменения регистров RTC
	RTC->CRL |= RTC_CRL_CNF; 								// Разрешить Запись в регистры RTC
	  RTC->CNTH = (FtimeToCounter(&Time))>>16;                                    // Записати нове значення годинникового лічильника
	  RTC->CNTL = FtimeToCounter(&Time);
	RTC->CRL &= ~RTC_CRL_CNF;                               // Запретить запись в регистры RTC
	while (!(RTC->CRL & RTC_CRL_RTOFF));                    // Дождаться окончания записи
	PWR->CR &= ~PWR_CR_DBP;                                 // Запретить доступ к Backup области
}


void RTC_Processing(void)
{

	uint32_t tmp;
	int i;

	tmp = (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
	RTC_Seconds = tmp % 60;

	if((!RTC_Seconds) || (one == 1))
	{
		one = 0;// шоб не чекати поки пройде хвилина

		RTC_Minutes = ((tmp / 60) % 60);
		RTC_Hours   = ((tmp / 3600) % 24);

		if (tmp % 86400)
		{
			RTC_Day=(tmp / 86400);
		}

		while (RTC_Day > 364)// кількість днів
			{
				if ((((RTC_Year - nulyear) % 4) == 0) && (RTC_Day == 366))  //якщо високосний
				{
					RTC_Day = RTC_Day - 366;
					RTC_Year++;
				}

				if (((RTC_Year - nulyear) % 4 != 0))    //якщо НЕ високосний
				{
					RTC_Day = RTC_Day - 365;
					RTC_Year++;
				}
			}

		if (((RTC_Year - year) % 4) == 0)
		{
			while (i < 12)
			{
				if (wys[i] < RTC_Day)
				{
					RTC_Month = i;
					i = 13;
				}
				else
				{
					RTC_Day = RTC_Day - wys[i];
					i++;
				}
			}
		}
		if (((RTC_Year - nulyear) % 4) != 0)
				{
					while (i < 12)
					{
						if (Nwys[i] < RTC_Day)
						{
							RTC_Month = i;
							i = 13;
						}
						else
						{
							RTC_Day = RTC_Day - Nwys[i];
							i++;
						}
					}
				}

		RTC_Month++;
		RTC_Day++;
	}
}

unsigned char RTC_GetSeconds(void)	{ return RTC_Seconds; }
unsigned char RTC_GetMinutes(void)	{ return RTC_Minutes; }
unsigned char RTC_GetHours(void)  	{ return RTC_Hours;   }
unsigned char RTC_GetDay(void)    	{ return RTC_Day;     }
unsigned char RTC_GetMonth(void)  	{ return RTC_Month;   }
unsigned int  RTC_GetYear(void)  	{ return RTC_Year;    }

void RTC_GetTimeStr(void)
{
	RTC_Processing();
	sprintf (RTC_TimeStr, "%d:%d:%d", RTC_Hours, RTC_Minutes, RTC_Seconds);
	sprintf (RTC_DateStr, "%d.%d.%d", RTC_Day, RTC_Month, RTC_Year);
}


void RTC_GetDateStr(void)
{
	char* yearStr;
	char* monthStr;
	char* dayStr;

	itoa(RTC_Year,  yearStr,   10);
	itoa(RTC_Month, monthStr, 10);
	itoa(RTC_Day,   dayStr, 10);

	return strcpy(strcpy(dayStr, " : "), strcpy(monthStr, strcpy(" : ", yearStr)));
}
