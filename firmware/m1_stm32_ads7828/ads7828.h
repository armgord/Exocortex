/*
 * ads7828.h - ADS7828 12-bit, 8-channel I2C ADC driver (STM32 HAL)
 * Part of the 8-channel EEG BCI (M1 discrete analog front end).
 */
#ifndef ADS7828_H
#define ADS7828_H

#include "stm32f4xx_hal.h"   /* change to match your Nucleo-64 family */
#include <stdint.h>

/* 7-bit address 0x48 with A1 = A0 = 0, shifted left for the HAL */
#define ADS7828_ADDR        (0x48 << 1)

/* Command byte fields (see ADS7828 datasheet) */
#define CMD_SD_SE           0x80   /* Single-ended inputs                 */
#define CMD_PD_EXT_REF_ON   0x04   /* Internal ref OFF, ADC ON (ext. ref) */

#define ADS7828_NUM_CH      8

HAL_StatusTypeDef ADS7828_Acquire_All(uint16_t *data_buffer);

#endif /* ADS7828_H */
