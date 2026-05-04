//
// Created by emmqa on 2026/2/5.
//

#ifndef SENSOR_MODULE_MODBUS_H
#define SENSOR_MODULE_MODBUS_H

#include "../../Core/Inc/main.h"
#include "../../Core/Inc/usart.h"

// ======== config =======
#define MODBUS_RE_DE_PIN     GPIO_PIN_3
#define MODBUS_RE_DE_PORT    GPIOA
#define MODBUS_UART          huart1
// =======================
#define MODBUS_Set_RE()      HAL_GPIO_WritePin(MODBUS_RE_DE_PORT, MODBUS_RE_DE_PIN, GPIO_PIN_RESET)
#define MODBUS_Set_TX()      HAL_GPIO_WritePin(MODBUS_RE_DE_PORT, MODBUS_RE_DE_PIN, GPIO_PIN_SET)

/* Command功能码 */
#define MODBUS_READ_HOLD_REGISTER      0x03     //读取保持寄存器
#define MODBUS_READ_INPUT_REGISTER     0x04     //读取输入寄存器
#define MODBUS_WRITE_HOLD_SINGLE_REGISTER      0x06     //写单个保持寄存器
#define MODBUS_WRITE_MULTIPLE_HOLD_REGISTER    0x10     //写多个保持寄存器



typedef enum
{
    MODBUS_OK = 0,
    MODBUS_ERROR,
    MODBUS_BUSY
} MODBUS_StatusTypedef;

/**
 * @brief 配置单个保持寄存器
 * @param SlaveAddress 从机地址
 * @param RegAddress 寄存器地址
 * @param Command 功能码
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_SetSingleHoldReg(uint8_t SlaveAddress, uint8_t RegAddress, uint8_t Command);

/**
 * @brief 配置多个保持寄存器
 * @param SlaveAddress 从机地址
 * @param RegBeginAddress 寄存器地址
 * @param RegNum 寄存器数量/长度
 * @param pData 数据缓冲区
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_WriteMultipleHoldRegs(uint8_t SlaveAddress, uint16_t RegBeginAddress,
                                                  uint16_t RegNum, uint16_t *pData);

/**
 * @brief 读输入寄存器
 * @param SlaveAddress 从机地址
 * @param RegBeginAddress 寄存器地址
 * @param ReadBuffer 读取数据缓冲区
 * @param RegNum 寄存器数量/长度
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_ReadInputReg(uint8_t SlaveAddress, uint16_t RegBeginAddress, uint8_t* ReadBuffer, uint16_t RegNum);



/**
 * @CRC校验函数
 * @param ptr 需要校验的帧
 * @param len 帧长度(包括CRC码)
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_CheckCRC16(uint8_t *ptr, uint16_t len);

/**
 * @brief 读保持寄存器
 * @param SlaveAddress: 从机地址
 * @param RegBeginAddress: 寄存器开始地址
 * @param ReadBuffer: 读取数据存储缓冲区
 * @param RegNum: 寄存器数量/长度
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_ReadHoldReg(uint8_t SlaveAddress, uint16_t RegBeginAddress, uint8_t* ReadBuffer, uint16_t RegNum);
#endif //SENSOR_MODULE_MODBUS_H