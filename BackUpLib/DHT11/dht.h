///////////////////////////////////////////////////
//	File:	dht11.h
//	Autor:	XSM
//	Date:	09.08.2018
///////////////////////////////////////////////////
#ifndef _DHT_H
#define _DHT_H

typedef struct
{
	char TERM[2];
	char null;
	char HUMID[2];
}DHT_DATA;

// Модель сенсора
#define DHT11                   11
#define DHT22                   22      // AM2302
#define DHT_MODEL               DHT11   // Модель сенсора (DHT11 или DHT22)

// Константы, обозначающие ошибки при обмене с DHT
#define DHT_ERR_Ok              0
#define DHT_ERR_NotConnect      -1
#define DHT_ERR_BadChksum       -2

#define DHT_WaitMaxTicks        1000    // Количество чтений GPIO, после которого считать, что фронт отсутствует

// Ноги, управляющие интерфейсом dht-11/21/22
#define DHT_Data_Port         GPIOA
#define DHT_Data_Pin          GPIO_Pin_5



// Функция считывает из DHT-11 5 байт данных в массив dht_Buff, проверяет целостность считанного пакета. Возвращает код ошибки
signed char dht_read_data(void);
// Функция возвращает предварительно считанное функцией dht_read_data() значение температуры из DHT-11.
float dht_get_temperature(void);
// Функция возвращает предварительно считанное функцией dht_read_data() значение влажности из DHT-11.
float dht_get_humidity(void);

#endif
