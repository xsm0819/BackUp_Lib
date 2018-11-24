//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "dht.h"
#include "delay.h"
#include "System.h"
#include <stdio.h>

// Макросы для работы с GPIO
#define DHT_Data_HIGH()       GPIO_SetBits(DHT_Data_Port, DHT_Data_Pin)
#define DHT_Data_LOW()        GPIO_ResetBits(DHT_Data_Port, DHT_Data_Pin)
#define DHT_Data_READ()       GPIO_ReadInputDataBit(DHT_Data_Port, DHT_Data_Pin)

int DHT_BitTimeTreshold = 0;
unsigned char dht_Buff[5];          // Буфер приёма из DHT

void SettRegisterDHT (void)
{
	RegisterMAP REG;
	unsigned char temp = dht_read_data();
	if (temp == DHT_ERR_Ok)
	{
		temp = dht_get_temperature();
		REG_TERM[1] = temp%10+0x30;
		REG_TERM[0] = temp/10+0x30;
		temp = dht_get_humidity();
		REG_HUM[1] = temp%10+0x30;
		REG_HUM[0] = temp/10+0x30;
	}
	else
	{
		REG_TERM[0] = 'E';
		REG_TERM[1] = 'R';
		REG_HUM[0] = 'E';
		REG_HUM[1] = 'R';
	}
}
//==============================================================================
// Процедура инициализирует пин микроконтроллера как вход/выход
//==============================================================================
void dht_DataPin_init(char IsOutput)
{
  // Стартуем тактирование периферии портов
  
  // Настраиваем ногу GPIO
  GPIO_InitTypeDef InitStruct;

  InitStruct.GPIO_OType = GPIO_OType_OD;
  InitStruct.GPIO_Mode = IsOutput ? GPIO_Mode_OUT : GPIO_Mode_IN;
  InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  InitStruct.GPIO_Pin = DHT_Data_Pin;
  GPIO_Init(DHT_Data_Port, &InitStruct);
}
//==============================================================================

//==============================================================================
// Функция ожидает фронт на линии Data. Если IsRising = 0 - фпронт задний, иначе
// - передний.
//==============================================================================
int dht_waitfront(char IsRising)
{
  int tick = 0;
    
  if (IsRising)
  {
    while ((!DHT_Data_READ()) && (tick < DHT_WaitMaxTicks))
      tick++;
  }
  else
  {
    while ((DHT_Data_READ()) && (tick < DHT_WaitMaxTicks))
      tick++;
  }
  
  return tick;
}
//==============================================================================


//==============================================================================
// Функция выставляет на пин Data пробуждающий DHT импульс и проверяет, выдал ли
// DHT свой подтверждающиц импульс в ответ
//==============================================================================
signed char dht_request(void)
{
  dht_DataPin_init(1);
  
  DHT_Data_LOW();
  delay_ms(22);
  DHT_Data_HIGH();
  
  dht_DataPin_init(0);
  
  dht_waitfront(0);
  int ticks1 = dht_waitfront(1);
  int ticks2 = dht_waitfront(0);
  
  if (ticks1 && ticks2 && (ticks1 < DHT_WaitMaxTicks) && (ticks2 < DHT_WaitMaxTicks))
  {
    DHT_BitTimeTreshold = ticks1 >> 1;
    return DHT_ERR_NotConnect;
  }
  
  return DHT_ERR_Ok;
}
//==============================================================================


//==============================================================================
// Функция считывает 1 бит данных из DHT
//==============================================================================
char dht_readbit(void)
{
  dht_waitfront(1);
  int ticks = dht_waitfront(0);
  
  if (ticks && (ticks < DHT_WaitMaxTicks))
    return (ticks > DHT_BitTimeTreshold) ? 1 : 0;
  
  return 0;
}
//==============================================================================


//==============================================================================
// Функция считает байт контрольной суммы по первым 4 байтам пакета из DHT
//==============================================================================
unsigned char dht_chkbyte(void)
{
  unsigned char ChkByte = 0;
  
  for (int i = 0; i < 4; i++)
    ChkByte += dht_Buff[i];
  
  return ChkByte;
}
//==============================================================================


//==============================================================================
// Функция считывает из DHT-11 5 байт данных в массив dht_Buff, проверяет 
// целостность считанного пакета. Возвращает код ошибки
//==============================================================================
signed char dht_read_data(void)
{
  for (int i = 0; i < 5; i++)
    dht_Buff[i] = 0;
  
  if (dht_request())
  {
    for (int i = 0; i < 40; i++)
    {
      if (dht_readbit())
      {
        char bitnum = 7 - (i % 8);
        dht_Buff[i >> 3] |= (1 << bitnum);
      }
    }
    
    if (dht_Buff[4] == dht_chkbyte())
      return DHT_ERR_Ok;
    else
      return DHT_ERR_BadChksum;
  }
  
  return DHT_ERR_NotConnect;
}
//==============================================================================


//==============================================================================
// Функция возвращает предварительно считанное функцией dht_read_data() значение 
// температуры из DHT-11.
//==============================================================================
float dht_get_temperature(void)
{
#if (DHT_MODEL == DHT22)
  float Val;
  char Val_Buff[2];
  Val_Buff[0] = dht_Buff[3];    // Младший байт int16 слова
  Val_Buff[1] = dht_Buff[2];    // Старший байт int16 слова
  Val = (*((uint16_t *)Val_Buff)) & 0x7FFF;
  Val /= 10;
  if (Val_Buff[0] & 0x80)       // Значение температуры отрицательное
    Val = -Val;
  return Val;
#else
  return *((signed char *)&dht_Buff[2]);
#endif
}
//==============================================================================


//==============================================================================
// Функция возвращает предварительно считанное функцией dht_read_data() значение 
// влажности из DHT-11.
//==============================================================================
float dht_get_humidity(void)
{
#if (DHT_MODEL == DHT22)
  float Val;
  char Val_Buff[2];
  Val_Buff[0] = dht_Buff[1];    // Младший байт int16 слова
  Val_Buff[1] = dht_Buff[0];    // Старший байт int16 слова
  Val = *((uint16_t *)Val_Buff);
  Val /= 10;
  return Val;
#else
  return *((signed char *)&dht_Buff[0]);
#endif
}
//==============================================================================
DHT_DATA DHT11_RESULT;

void ConvertDHT (void)
{
	if (dht_read_data()==0)
	{
		char temp = dht_get_temperature();

		DHT11_RESULT.TERM[0]=temp/10+0x30;
		DHT11_RESULT.TERM[1]=temp%10+0x30;
		temp = dht_get_humidity();

		DHT11_RESULT.HUMID[0]=temp/10+0x30;
		DHT11_RESULT.HUMID[1]=temp%10+0x30;
	}
}
