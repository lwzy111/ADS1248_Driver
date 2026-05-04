#include "callbacks.h"


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    // SPI1 设备
    if (hspi == &hspi1) {
        // 判断是哪个设备（通过 CS 状态或上下文）

        return;
    }

}