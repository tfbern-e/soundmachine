/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    callbacks.c
  * @brief   Callback Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Technische Fachschule Bern
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by TFbern under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pdm2pcm_glo.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim2;
/* USER CODE BEGIN EV */
//external handlers
extern DAC_HandleTypeDef hdac1;
extern UART_HandleTypeDef huart1;

//audio
extern uint8_t pdmbuffer[PDMBUFFERSIZE];
extern uint8_t pdmtestbuffer[PDMBUFFERSIZE];
extern uint16_t pcmbuffer[PCMBUFFERSIZE];
extern uint8_t PDMBufferReady;
extern uint8_t PDMReadyBufferIndex;
extern uint8_t PCMBufferReady;
extern uint8_t PCMReadyBufferIndex;
extern PDM_Filter_Handler_t FilterHandler;
extern uint8_t pdmtestbuffer[PDMBUFFERSIZE];

//flash
extern uint8_t FlashWriteBuffer[];
extern uint32_t FlashWriteBufferIndex;
extern uint8_t WriteDoubleBufferIndex;
extern uint8_t FlashReadBuffer[];
extern uint16_t FlashReadBufferIndex;
extern uint8_t ReadDoubleBufferIndex;

//mic
extern uint8_t MicWokeUp;
/* USER CODE END EV */

/* USER CODE BEGIN 1 */
/**
  * @brief Rx Transfer half completed callback.
  * @param  hsai pointer to a SAI_HandleTypeDef structure that contains
  *              the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
	uint8_t PCMBufferIndex;
	
	//sai 1. half buffer		
	if (MicWokeUp == TRUE)
	{
		//HAL_GPIO_WritePin(H3_GPIO_Port, H3_Pin, GPIO_PIN_SET);
		
		//low pdm buffer to pcm buffer
		PDM_Filter(&pdmbuffer[0], &pcmbuffer[0], &FilterHandler);
		
		//copy pcm buffer to flash buffer
		for (PCMBufferIndex = 0 ; PCMBufferIndex < PCMBUFFERSIZE; PCMBufferIndex++)
		{
			FlashWriteBuffer[FlashWriteBufferIndex] = (uint8_t)((pcmbuffer[PCMBufferIndex]&0xff00)>>8);
			FlashWriteBufferIndex++;
			//16 Bit
			//FlashWriteBuffer[FlashWriteBufferIndex] = ((pcmbuffer[PCMBufferIndex]&0x00ff));				
			//FlashWriteBufferIndex++;
		}
				
		if (FlashWriteBufferIndex == FLASHPAGESIZE)
		{
			WriteDoubleBufferIndex = DOUBLEBUFFER_LOW;
		}
		else if (FlashWriteBufferIndex == (FLASHPAGESIZE * 2))
		{
			WriteDoubleBufferIndex = DOUBLEBUFFER_HIGH;
			FlashWriteBufferIndex = 0;
		}
		
		//HAL_GPIO_WritePin(H3_GPIO_Port, H3_Pin, GPIO_PIN_RESET);
		
		#ifdef DEBUG
		HAL_UART_Transmit(&huart1, &pdmbuffer[0], PDMBUFFERSIZE/2, 100);
		#endif
	}	
}

/**
  * @brief Rx Transfer completed callback.
  * @param  hsai pointer to a SAI_HandleTypeDef structure that contains
  *              the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
	uint8_t PCMBufferIndex;	
	
	//spi1 2. half buffer		
	if (MicWokeUp == TRUE)
	{
		//HAL_GPIO_WritePin(H4_GPIO_Port, H4_Pin, GPIO_PIN_SET);
		
		//high pdm buffer to pcm buffer
		PDM_Filter(&pdmbuffer[PDMBUFFERSIZE/2], &pcmbuffer[0], &FilterHandler);
		
		//copy pcm buffer to flash buffer
		for (PCMBufferIndex = 0 ; PCMBufferIndex < PCMBUFFERSIZE; PCMBufferIndex++){
			FlashWriteBuffer[FlashWriteBufferIndex] = (uint8_t)((pcmbuffer[PCMBufferIndex]&0xff00)>>8);
			FlashWriteBufferIndex++;
			//16 Bit
			//FlashWriteBuffer[FlashWriteBufferIndex] = ((pcmbuffer[PCMBufferIndex]&0x00ff));				
			//FlashWriteBufferIndex++;
		}

		if (FlashWriteBufferIndex == FLASHPAGESIZE)
		{
			WriteDoubleBufferIndex = DOUBLEBUFFER_LOW;
		}
		else if (FlashWriteBufferIndex == (FLASHPAGESIZE * 2))
		{
			WriteDoubleBufferIndex = DOUBLEBUFFER_HIGH;
			FlashWriteBufferIndex = 0;
		}			
		
		//HAL_GPIO_WritePin(H4_GPIO_Port, H4_Pin, GPIO_PIN_RESET);
		
		#ifdef DEBUG
		HAL_UART_Transmit(&huart1, &pdmbuffer[PDMBUFFERSIZE/2], PDMBUFFERSIZE/2, 100);	
		#endif
	}	
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		//flash CS high
		HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
		
		//HAL_GPIO_WritePin(H6_GPIO_Port, H6_Pin, GPIO_PIN_RESET);
	}
}

/**
  * @brief  Tx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		//flash CS high
		HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
		
		//HAL_GPIO_WritePin(H5_GPIO_Port, H5_Pin, GPIO_PIN_RESET);
	}
}
/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	uint32_t ActualAudioValue;
	
	if (htim->Instance  == TIM2)
	{
		ActualAudioValue = /*( ((uint32_t)FlashReadBuffer[FlashReadBufferIndex]) << 8 ) &*/ (uint32_t)FlashReadBuffer[FlashReadBufferIndex];
		
		HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R/*DAC_ALIGN_12B_R*/, ActualAudioValue);
		
		FlashReadBufferIndex++;
		
		if (FlashReadBufferIndex == FLASHPAGESIZE)
		{
			ReadDoubleBufferIndex = DOUBLEBUFFER_LOW;
		}
		else if (FlashReadBufferIndex == (FLASHPAGESIZE * 2))
		{
			ReadDoubleBufferIndex = DOUBLEBUFFER_HIGH;
			FlashReadBufferIndex = 0;
		}
	}
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT TFbern *****END OF FILE****/
