#include "main.h"
#include "i2c.h"
#include "sdk_uart.h"
#include "pca9538.h"

HAL_StatusTypeDef PCA9538_Read_Register(uint16_t addr, pca9538_regs_t reg, uint8_t* buf) {
  return HAL_I2C_Mem_Read(&hi2c1, addr | 1, reg, 1, buf, 1, 100);
}


HAL_StatusTypeDef PCA9538_Write_Register(uint16_t addr, pca9538_regs_t reg, uint8_t* buf) {
  return HAL_I2C_Mem_Write(&hi2c1, addr & 0xFFFE, reg, 1, buf, 1, 100);
}

HAL_StatusTypeDef PCA9538_Read_Config(uint16_t addr, uint8_t* buf) {
  HAL_StatusTypeDef ret = HAL_OK;
  uint8_t *buf_ptr = buf;
  uint8_t i;

  for( i=0; i<4; i++ ) {
    ret = PCA9538_Read_Register(addr, i, buf_ptr++);

    if(ret!=HAL_OK)
      return ret;
  }

  return ret;
}

HAL_StatusTypeDef PCA9538_Check_DefaultConfig(uint16_t addr) {
  HAL_StatusTypeDef ret = HAL_OK;
  uint8_t buf[4];

  ret = PCA9538_Read_Config(addr, buf);
  if( ret != HAL_OK ){
    UART_Transmit((uint8_t*)"Can't read default config\n");
  }
  else if ( buf[1] != 0xFF && buf[2] != 0xFF && buf[3] != 0xFF ) {
    UART_Transmit((uint8_t*)"Non-reset config\n");
    UART_Transmit(buf);
    ret = HAL_ERROR;
  }

  return ret;
}

HAL_StatusTypeDef PCA9538_Read_Inputs(uint16_t addr, uint8_t* buf) {
  return PCA9538_Read_Register(addr, INPUT_PORT, buf);
}

HAL_StatusTypeDef PCA9538_OLED_Reset(void) {
  HAL_StatusTypeDef ret;
  uint8_t cfg = 0xFF;
  uint8_t out = 0xFF;

  /* Read configuration of DIP/OLED PCA9538 (0xE0) and configure pin 0 as output (0) */
  if (PCA9538_Read_Register(DIP_WR_ADDR, CONFIG, &cfg) != HAL_OK) {
    cfg = 0xFE;
  } else {
    cfg &= ~(1u << 0);
  }
  ret = PCA9538_Write_Register(DIP_WR_ADDR, CONFIG, &cfg);
  if (ret != HAL_OK) {
    return ret;
  }

  /* Read output register or default to 0xFF */
  if (PCA9538_Read_Register(DIP_WR_ADDR, OUTPUT_PORT, &out) != HAL_OK) {
    out = 0xFF;
  }

  /* 1. Drive pin 0 HIGH */
  out |= (1u << 0);
  ret = PCA9538_Write_Register(DIP_WR_ADDR, OUTPUT_PORT, &out);
  if (ret != HAL_OK) {
    return ret;
  }
  HAL_Delay(1);

  /* 2. Pulse pin 0 LOW (active low reset, min 1 us pulse) */
  out &= ~(1u << 0);
  ret = PCA9538_Write_Register(DIP_WR_ADDR, OUTPUT_PORT, &out);
  if (ret != HAL_OK) {
    return ret;
  }
  HAL_Delay(1);

  /* 3. Return pin 0 HIGH (release reset) and wait >= 1 ms */
  out |= (1u << 0);
  ret = PCA9538_Write_Register(DIP_WR_ADDR, OUTPUT_PORT, &out);
  if (ret != HAL_OK) {
    return ret;
  }
  HAL_Delay(2);

  return HAL_OK;
}