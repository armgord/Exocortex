/*
 * ads7828.c - Sequential read of all 8 EEG channels from the ADS7828 over I2C.
 *
 * Status: early polling implementation. Planned next steps are timer-triggered
 * sampling and interrupt/DMA-driven I2C transfers so no samples are dropped.
 */
#include "ads7828.h"

extern I2C_HandleTypeDef hi2c1;

uint16_t eeg_raw_data[ADS7828_NUM_CH];

/* Channel-select bits (C2 C1 C0) in the order the ADS7828 maps them
 * for single-ended mode: CH0=0x00, CH1=0x40, CH2=0x10, CH3=0x50, ... */
static const uint8_t channel_map[ADS7828_NUM_CH] = {
    0x00, 0x40, 0x10, 0x50, 0x20, 0x60, 0x30, 0x70
};

HAL_StatusTypeDef ADS7828_Acquire_All(uint16_t *data_buffer)
{
    uint8_t command_byte;
    uint8_t rx_buffer[2];
    HAL_StatusTypeDef status;

    for (int i = 0; i < ADS7828_NUM_CH; i++) {
        /* 1. Command: single-ended + channel + external reference */
        command_byte = CMD_SD_SE | channel_map[i] | CMD_PD_EXT_REF_ON;

        status = HAL_I2C_Master_Transmit(&hi2c1, ADS7828_ADDR, &command_byte, 1,
                                         HAL_MAX_DELAY);
        if (status != HAL_OK) return status;

        /* 2. Read the conversion result */
        status = HAL_I2C_Master_Receive(&hi2c1, ADS7828_ADDR, rx_buffer, 2,
                                        HAL_MAX_DELAY);
        if (status != HAL_OK) return status;

        /* 3. Reconstruct 12-bit value: [0000 D11-D8] [D7-D0] */
        data_buffer[i] = ((rx_buffer[0] & 0x0F) << 8) | rx_buffer[1];
    }

    return HAL_OK;
}
