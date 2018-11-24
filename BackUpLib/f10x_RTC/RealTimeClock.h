////////////////////////////////////////////////////////////////////////////////
// File:   RealTimeClock.h
// Author: Illarionovich
//
// Created on 17 06 2016, 17:56
////////////////////////////////////////////////////////////////////////////////

#ifndef REALTIMECLOCK_H
#define REALTIMECLOCK_H

#include "inttypes.h"
char RTC_TimeStr[10], RTC_DateStr[11];

void RTC_IRQ(void);
void RTC_Init(void);
void RTC_Setup(unsigned char sec,	unsigned char min, unsigned char hours, unsigned char day, unsigned char mouh, unsigned int year);

unsigned char RTC_GetSeconds(void);
unsigned char RTC_GetMinutes(void);
unsigned char RTC_GetHours(void);
unsigned char RTC_GetDay(void);
unsigned char RTC_GetMonth(void);
unsigned int  RTC_GetYear(void);


void RTC_GetTimeStr(void);
void RTC_GetDateStr(void);

#define JD0 2451911 // днів до 01 січня 2001 ПН

typedef struct{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} ftime_t;

// функція перетворення григоріанської дати і часу в значення лічильника
uint32_t FtimeToCounter(ftime_t * ftime);
unsigned char time;

#endif /* REALTIMECLOCK_H */
