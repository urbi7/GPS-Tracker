/*
 ******************************************************************************
 * @file    free_fall.c
 * @author  Sensors Software Solution Team
 * @brief   This file show the simplest way to detect free fall events
 *          from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
#define SENSOR_BUS hi2c2
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "lis2dw12_reg.h"
#include "usart.h"
#include "gpio.h"
#include "i2c.h"
#include "stm32l4xx_hal.h"
//GSM
#include "gsm.h"
/* Private macro -------------------------------------------------------------*/
#define    BOOT_TIME            20 //ms
/* Private variables ---------------------------------------------------------*/
static uint8_t whoamI, rst;
static uint8_t tx_buffer[1000];
/* Extern variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_delay(uint32_t ms);
/* Main Example --------------------------------------------------------------*/
void lis2dw12_free_fall(void)
{
  /* Initialize mems driver interface */
  stmdev_ctx_t dev_ctx;
  lis2dw12_reg_t int_route;
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.mdelay = platform_delay;
  dev_ctx.handle = &SENSOR_BUS;
  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);
  /* Check device ID */
  lis2dw12_device_id_get(&dev_ctx, &whoamI);

  if (whoamI != LIS2DW12_ID)
    {
	  platform_delay(100);
      sprintf((char*)tx_buffer, "Nie wykryto akcelerometru! %d\n\r", whoamI);
      tx_com(tx_buffer, strlen((char const*)tx_buffer));
      while (1);
    }

  /* Restore default configuration */
  lis2dw12_reset_set(&dev_ctx, PROPERTY_ENABLE);

  do {
    lis2dw12_reset_get(&dev_ctx, &rst);
  } while (rst);

  /* Configure power mode */
  lis2dw12_power_mode_set(&dev_ctx, LIS2DW12_HIGH_PERFORMANCE_LOW_NOISE);
  /* Set Output Data Rate */
  lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_200Hz);
  /* Set full scale to 2 g */
  lis2dw12_full_scale_set(&dev_ctx, LIS2DW12_2g);
  /* Configure Free Fall duration and samples count */
  lis2dw12_ff_dur_set(&dev_ctx, 0x06);
  lis2dw12_ff_threshold_set(&dev_ctx, LIS2DW12_FF_TSH_10LSb_FS2g);
  /* Enable free fall interrupt */
  lis2dw12_pin_int1_route_get(&dev_ctx, &int_route.ctrl4_int1_pad_ctrl);
  int_route.ctrl4_int1_pad_ctrl.int1_ff = PROPERTY_ENABLE;
  lis2dw12_pin_int1_route_set(&dev_ctx, &int_route.ctrl4_int1_pad_ctrl);
  /* Set latched interrupt */
  lis2dw12_int_notification_set(&dev_ctx, LIS2DW12_INT_LATCHED);

  /* Wait Events */
  while (1) {
    /* Check Free Fall events */
    lis2dw12_all_sources_t src;
    lis2dw12_all_sources_get(&dev_ctx, &src);

    if (src.wake_up_src.ff_ia) {
      sprintf((char *)tx_buffer, "Wykryto upadek! Uruchomiono brzeczyk oraz wyslano wiadomosc SMS.\r\n");
      tx_com(tx_buffer, strlen((char const *)tx_buffer));

      HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
      HAL_Delay(1000);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
	  //GSM
	  gsm_msg_send("+48669631922", "Wykryto upadek!");

	  lis2dw12_all_sources_get(&dev_ctx, &src);
	  platform_delay(1000);
    }

    platform_delay(100);
  }
}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
  HAL_I2C_Mem_Write(handle, LIS2DW12_I2C_ADD_H, reg,
                    I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  HAL_I2C_Mem_Read(handle, LIS2DW12_I2C_ADD_H, reg,
                   I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);

  return 0;
}

static void tx_com(uint8_t *tx_buffer, uint16_t len)
{
  HAL_UART_Transmit(&hlpuart1, tx_buffer, len, 1000);
  HAL_Delay(100);
}

static void platform_delay(uint32_t ms)
{
  HAL_Delay(ms);
}
