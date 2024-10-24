/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "gpio.h"
#include "i2c.h"
//ACC
#include "lis2dw12_reg.h"
//GSM
#include "gsm.h"
//GPS
#include "uartRingBuffer.h"
#include "NMEA.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
//GPS
char GGA[100];
char RMC[100];
GPSSTRUCT gpsData;
int flagGGA = 0, flagRMC = 0;
int VCCTimeout = 5000;
/* USER CODE END Variables */
/* Definitions for TaskGPS */
osThreadId_t TaskGPSHandle;
const osThreadAttr_t TaskGPS_attributes = {
  .name = "TaskGPS",
  .stack_size = 756 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskGSM */
osThreadId_t TaskGSMHandle;
const osThreadAttr_t TaskGSM_attributes = {
  .name = "TaskGSM",
  .stack_size = 756 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskFF */
osThreadId_t TaskFFHandle;
const osThreadAttr_t TaskFF_attributes = {
  .name = "TaskFF",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void lis2dw12_free_fall();
/* USER CODE END FunctionPrototypes */

void StartTaskGPS(void *argument);
void StartTaskGSM(void *argument);
void StartTaskFF(void *argument);

extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  Ringbuf_init();
  osDelay(500);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskGPS */
  TaskGPSHandle = osThreadNew(StartTaskGPS, NULL, &TaskGPS_attributes);

  /* creation of TaskGSM */
  TaskGSMHandle = osThreadNew(StartTaskGSM, NULL, &TaskGSM_attributes);

  /* creation of TaskFF */
  TaskFFHandle = osThreadNew(StartTaskFF, NULL, &TaskFF_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTaskGPS */
/**
  * @brief  Function implementing the TaskGPS thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskGPS */
void StartTaskGPS(void *argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN StartTaskGPS */
  /* Infinite loop */
  for(;;)
  {
	    if (Wait_for("GGA") == 1)
	    {
	    	VCCTimeout = 5000;
	    	Copy_upto("*", GGA);
	    	if(decodeGGA(GGA, &gpsData.ggastruct)== 0) flagGGA = 2;
	    	else flagGGA = 1;
	    }

		if (Wait_for("RMC") == 1)
		{
			VCCTimeout = 5000;
			Copy_upto("*", RMC);
			if(decodeRMC(RMC, &gpsData.rmcstruct)== 0) flagRMC = 2;
			else flagRMC = 1;
		}

		if ((flagGGA == 2) | (flagRMC == 2))
		{
			printf("Godzina: %02d:%02d:%02d, Data: %02d.%02d.%02d \r\n", gpsData.ggastruct.tim.hour, gpsData.ggastruct.tim.min, gpsData.ggastruct.tim.sec, gpsData.rmcstruct.date.Day, gpsData.rmcstruct.date.Mon, gpsData.rmcstruct.date.Yr);
			printf("Lokalizacja: %.2f%c, %.2f%c  \r\n", gpsData.ggastruct.lcation.latitude, gpsData.ggastruct.lcation.NS,gpsData.ggastruct.lcation.longitude, gpsData.ggastruct.lcation.EW);
		}
		else if ((flagGGA == 1) | (flagRMC == 1))
		{
			printf("Brak FIXa, prosze czekac.\r\n");
		}

	    if (VCCTimeout <= 0)
		{
			VCCTimeout = 5000;
			flagGGA =flagRMC =0;
			printf("Problem z zasilaniem, sprawdz polaczenie.\r\n");
		}
    osDelay(5000);
  }
  /* USER CODE END StartTaskGPS */
}

/* USER CODE BEGIN Header_StartTaskGSM */
/**
* @brief Function implementing the TaskGSM thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskGSM */
void StartTaskGSM(void *argument)
{
  /* USER CODE BEGIN StartTaskGSM */

  /* Infinite loop */
	gsm_init();
	gsm_waitForRegister(30);
  for(;;)
  {
	gsm_loop();
	osDelay(1000);
  }
  /* USER CODE END StartTaskGSM */
}

/* USER CODE BEGIN Header_StartTaskFF */
/**
* @brief Function implementing the TaskFF thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskFF */
void StartTaskFF(void *argument)
{
  /* USER CODE BEGIN StartTaskFF */
  /* Infinite loop */
  for(;;)
  {
	lis2dw12_free_fall();
	osDelay(500);
  }
  /* USER CODE END StartTaskFF */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

