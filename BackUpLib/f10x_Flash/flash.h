//////////////////////////////////////////////////
///     File:           		flash.h
///     Projekt:        ODS
///     Autor:          	XSM
///     Date:           	05.04.2018		13:35
//////////////////////////////////////////////////


#ifndef FLASH_H
#define FLASH_H

#include "stm32f10x_flash.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"

#define MY_FLASH_PAGE_ADDR 0x800FC00

typedef struct
  {

                            // 8 byte = 2  32-bits words.  It's - OK
                            // !!! Full size (bytes) must be a multiple of 4 !!!
  } tpSettings;




  void FLASH_Init(void) ;
  void FLASH_ReadSettings(void);
  void FLASH_WriteSettings(void);

#endif	/*		FLASH_H	*/
