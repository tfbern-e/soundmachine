/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash.h
  * @brief          : Header for flash.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 Technische Fachschule Bern.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by TF-Bern under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define FLASHPAGESIZE 0x100 							//256 bytes		(Addresssize: 0x100)
#define FLASHSECTORSIZE 16*FLASHPAGESIZE	//4096 bytes	(Addresssize: 0x1000)
#define FLASH32KBLOCK	8*FLASHSECTORSIZE		//32768 bytes (Addresssize: 0x8000)
#define FLASH64KBLOCK	2*FLASH32KBLOCK 		//65536 bytes	(Addresssize: 0x010000)

#define FLASH_HEADER_SIZE FLASHPAGESIZE
#define FLASH_AUDIO_SIZE FLASHSECTORSIZE
//#define FLASH_AUDIO_SIZE 0xff00 //15 Sectors & 15 Pages --> mit HEADER zusammen = 1 x FLASH64KBLOCK

//Speicherbereich pro Channel = 64 Blocks = 1/4 des gesammten Flash
#define FLASHCH1STARTADDRESS 0*64*FLASH64KBLOCK
#define FLASHCH2STARTADDRESS 1*64*FLASH64KBLOCK
#define FLASHCH3STARTADDRESS 2*64*FLASH64KBLOCK
#define FLASHCH4STARTADDRESS 3*64*FLASH64KBLOCK
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
uint8_t Flash_WriteEnable(void);
uint8_t Flash_WriteDisable(void);
uint8_t Flash_WriteInProgess(void);
uint8_t Flash_4KBSectorErase(uint32_t SectorAdress);
uint8_t Flash_64KBBlockErase(uint32_t Address);
uint8_t Flash_WriteData(uint32_t PageAdress, uint32_t DataSize, uint8_t *Data);
uint8_t Flash_WriteData_DMA(uint32_t PageAdress, uint32_t DataSize, uint8_t *Data);
uint8_t Flash_ReadData(uint32_t Adress, uint16_t DataSize, uint8_t * Data);
uint8_t Flash_ReadData_DMA(uint32_t Address, uint16_t DataSize, uint8_t * Data);
uint8_t Flash_ReadStatus(uint8_t StatusRegister);
uint16_t Flash_ReadManufacturerDevID(void);
void FlashTest(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H */

/************************ (C) COPYRIGHT Technische Fachschule Bern *****END OF FILE****/
