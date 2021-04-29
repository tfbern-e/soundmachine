/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : soundmachine.h
  * @brief          : Additional Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 Technische Fachschule Bern.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOUNDMACHINE_H
#define __SOUNDMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

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

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define PLAY 0
#define RECORD 1
#define PDMBUFFERSIZE 256*2
#define PCMBUFFERSIZE 32
#define TRUE 1
#define FALSE 0
#define DOUBLEBUFFER_NONE 0
#define DOUBLEBUFFER_LOW 1
#define DOUBLEBUFFER_HIGH 2

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __SOUNDMACHINE_H */

/************************ (C) COPYRIGHT TFbern *****END OF FILE****/
