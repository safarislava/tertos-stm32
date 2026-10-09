#include "main.h"
#include "pca9538.h"
#include "kb.h"
#include "sdk_uart.h"

#define KBRD_ADDR 0xE2

HAL_StatusTypeDef Set_Keyboard(void) {
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t buf;

    buf = 0;
    ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write polarity\n");
        goto exit;
    }

    buf = 0;
    ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write output\n");
    }

exit:
    return ret;
}

enum Button Check_Row(uint8_t row) {
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t buf;
    uint8_t in;

    ret = Set_Keyboard();
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write init\n");
        return B_NONE;
    }

    buf = row;
    ret = PCA9538_Write_Register(KBRD_ADDR, CONFIG, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write config\n");
        return B_NONE;
    }

    ret = PCA9538_Read_Inputs(KBRD_ADDR, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Read error\n");
        return B_NONE;
    }

    in = buf & 0x70;
    if (in != 0x70) {
        if (!(in & 0x10)) {
            if (row == ROW1) return B_PAUSE;
            if (row == ROW2) return B_LEFT;
            if (row == ROW3) return B_NONE;
            if (row == ROW4) return B_NONE;
        }
        if (!(in & 0x20)) {
            if (row == ROW1) return B_UP;
            if (row == ROW2) return B_NONE;
            if (row == ROW3) return B_DOWN;
            if (row == ROW4) return B_NONE;
        }
        if (!(in & 0x40)) {
            if (row == ROW1) return B_NONE;
            if (row == ROW2) return B_RIGHT;
            if (row == ROW3) return B_NONE;
            if (row == ROW4) return B_NONE;
        }
    }

    return B_NONE;
}

enum Button Get_Char(void) {
    static enum Button last_key = B_NONE;
    uint8_t rows[4] = {ROW1, ROW2, ROW3, ROW4};
    enum Button current_key = B_NONE;

    for (int i = 0; i < 4; i++) {
        enum Button key = Check_Row(rows[i]);
        if (key != B_NONE) {
            current_key = key;
            break;
        }
    }

    if (current_key != B_NONE && current_key != last_key) {
        last_key = current_key;
        HAL_Delay(20);
        return current_key;
    }
    if (current_key == B_NONE) {
        last_key = B_NONE;
    }

    return B_NONE;
}