//
// Created by emmqa on 2026/2/5.
//
#include "../Inc/modbus.h"

MODBUS_StatusTypedef MODBUS_Transmit(uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    while (MODBUS_UART.gState != HAL_UART_STATE_READY);  // Wait until UART is ready
    MODBUS_Set_TX();  // Set to transmit mode
    HAL_Delay(2);
    HAL_StatusTypeDef status = HAL_UART_Transmit(&MODBUS_UART, pData, Size, Timeout);
    // TC (Transmission Complete) 标志位清零表示移位寄存器已空
    while(__HAL_UART_GET_FLAG(&MODBUS_UART, UART_FLAG_TC) == RESET);

    MODBUS_Set_RE();  // 数据发完后切换回接收模式
    return (status == HAL_OK) ? MODBUS_OK : MODBUS_ERROR;
}

MODBUS_StatusTypedef MODBUS_Receive(uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    while (MODBUS_UART.gState != HAL_UART_STATE_READY);
    MODBUS_Set_RE();
    if (HAL_UART_Receive(&MODBUS_UART, pData, Size, Timeout) == HAL_OK)
    {
        return MODBUS_OK;
    }
    else
    {
        return MODBUS_ERROR;
    }
}

/**
 * @CRC校验码计算函数
 * @param ptr 需要计算CRC码的帧
 * @param len 帧的长度
 * @return crc
 */
uint16_t MODBUS_CalculateCRC16(uint8_t *ptr, uint8_t len) {
    uint16_t crc = 0xFFFF;
    while(len--) {
        crc ^= *ptr++;
        for(int i = 0; i < 8; i++) {
            if(crc & 0x01) { crc >>= 1; crc ^= 0xA001; }
            else { crc >>= 1; }
        }
    }
    return crc;
}

/**
 * @CRC校验函数
 * @param ptr 需要校验的帧
 * @param len 帧长度(包括CRC码)
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_CheckCRC16(uint8_t *ptr, uint16_t len)
{
    uint16_t calculated_crc = MODBUS_CalculateCRC16(ptr, len - 2);
    uint16_t received_crc = ptr[len - 1] << 8 | ptr[len - 2];
    if(received_crc == calculated_crc)
    {
        return MODBUS_OK;
    }
    else
    {
        return MODBUS_ERROR;
    }
}


/**
 * @brief 配置单个保持寄存器
 * @param SlaveAddress: 从机地址
 * @param RegAddress: 寄存器地址
 * @param Command: 命令
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_SetSingleHoldReg(uint8_t SlaveAddress, uint8_t RegAddress, uint8_t Command)
{
    uint8_t sendBuffer[8];
    sendBuffer[0] = SlaveAddress;
    sendBuffer[1] = MODBUS_WRITE_HOLD_SINGLE_REGISTER;
    sendBuffer[2] = 0x00;
    sendBuffer[3] = RegAddress;
    sendBuffer[4] = 0x00;
    sendBuffer[5] = Command;

    uint16_t crc = MODBUS_CalculateCRC16(sendBuffer, 6);
    sendBuffer[6] = crc & 0xFF;          // CRC低位在前
    sendBuffer[7] = crc >> 8;            // CRC高位在后
    if (MODBUS_Transmit(sendBuffer, 8, 100) != MODBUS_OK) return MODBUS_ERROR;

    uint8_t readBuffer[8];
    MODBUS_Receive(readBuffer, 8, 100);

    for (uint8_t i = 0; i < 8; i++)
    {
        if (sendBuffer[i] != readBuffer[i]) return MODBUS_ERROR;
    }

    return MODBUS_OK;
}

/**
 * @brief 配置多个保持寄存器
 * @param SlaveAddress: 从机地址
 * @param RegAddress: 寄存器地址
 * @param Command: 命令
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_WriteMultipleHoldRegs(uint8_t SlaveAddress, uint16_t RegBeginAddress,
                                                  uint16_t RegNum, uint16_t *pData)
{
    uint16_t datalen = RegNum * 2;
    uint16_t packetlen = 7 + datalen;
    uint8_t sendBuffer[256];

    if (packetlen + 2 > 256) return MODBUS_ERROR;

    sendBuffer[0] = SlaveAddress;
    sendBuffer[1] = MODBUS_WRITE_MULTIPLE_HOLD_REGISTER;
    sendBuffer[2] = RegBeginAddress >> 8;
    sendBuffer[3] = RegBeginAddress & 0xFF;
    sendBuffer[4] = RegNum >> 8;
    sendBuffer[5] = RegNum & 0xFF;
    sendBuffer[6] = datalen;

    for (uint16_t i = 0; i < RegNum; i++)
    {
        sendBuffer[ 7 + i * 2] = (uint8_t)(pData[i] >> 8);
        sendBuffer[ 8 + i * 2] = (uint8_t)(pData[i] & 0xFF);
    }

    uint16_t crc = MODBUS_CalculateCRC16(sendBuffer, packetlen);
    sendBuffer[7 + datalen] = (uint8_t)(crc & 0xFF);          // CRC低位在前
    sendBuffer[8 + datalen] = (uint8_t)(crc >> 8);            // CRC高位在后

    if (MODBUS_Transmit(sendBuffer, packetlen + 2, 100) != MODBUS_OK) return MODBUS_ERROR;

    uint8_t readBuffer[8];
    if (MODBUS_Receive(readBuffer, 8, 100) != MODBUS_OK) return MODBUS_ERROR;

    for (uint8_t i = 0; i < 6; i++)
    {
        if (sendBuffer[i] != readBuffer[i]) return MODBUS_ERROR;
    }

    // 校验响应帧的 CRC
    uint16_t resCRC = MODBUS_CalculateCRC16(readBuffer, 6);
    if (((uint8_t)(resCRC & 0xFF) != readBuffer[6]) || ((uint8_t)(resCRC >> 8) != readBuffer[7]))
    {
        return MODBUS_ERROR;
    }

    return MODBUS_OK;
}

/**
 * @brief 读输入寄存器
 * @param SlaveAddress: 从机地址
 * @param RegBeginAddress: 寄存器开始地址
 * @param ReadBuffer: 读取数据存储缓冲区
 * @param RegNum: 寄存器数量/长度
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_ReadInputReg(uint8_t SlaveAddress, uint16_t RegBeginAddress, uint8_t* ReadBuffer, uint16_t RegNum)
{

    uint8_t sendBuffer[8];
    uint8_t expectedRxlen = RegNum * 2 + 5;
    MODBUS_StatusTypedef status;

    sendBuffer[0] = SlaveAddress;
    sendBuffer[1] = MODBUS_READ_INPUT_REGISTER;
    sendBuffer[2] = RegBeginAddress >> 8;
    sendBuffer[3] = RegBeginAddress & 0xFF;
    sendBuffer[4] = RegNum >> 8;
    sendBuffer[5] = RegNum & 0xFF;

    uint16_t crc = MODBUS_CalculateCRC16(sendBuffer, 6);
    sendBuffer[6] = crc & 0xFF;
    sendBuffer[7] = crc >> 8;

    memset(ReadBuffer, 0, expectedRxlen);
    status = MODBUS_Transmit(sendBuffer, 8, 100);

    if (status != MODBUS_OK) return MODBUS_ERROR;
    return MODBUS_Receive(ReadBuffer, expectedRxlen, 100);
}

/**
 * @brief 读保持寄存器
 * @param SlaveAddress: 从机地址
 * @param RegBeginAddress: 寄存器开始地址
 * @param ReadBuffer: 读取数据存储缓冲区
 * @param RegNum: 寄存器数量/长度
 * @return MODBUS_StatusTypedef
 */
MODBUS_StatusTypedef MODBUS_ReadHoldReg(uint8_t SlaveAddress, uint16_t RegBeginAddress, uint8_t* ReadBuffer, uint16_t RegNum)
{

    uint8_t sendBuffer[8];
    uint8_t expectedRxlen = RegNum * 2 + 5;
    MODBUS_StatusTypedef status;

    sendBuffer[0] = SlaveAddress;
    sendBuffer[1] = MODBUS_READ_HOLD_REGISTER;
    sendBuffer[2] = RegBeginAddress >> 8;
    sendBuffer[3] = RegBeginAddress & 0xFF;
    sendBuffer[4] = RegNum >> 8;
    sendBuffer[5] = RegNum & 0xFF;

    uint16_t crc = MODBUS_CalculateCRC16(sendBuffer, 6);
    sendBuffer[6] = crc & 0xFF;
    sendBuffer[7] = crc >> 8;

    memset(ReadBuffer, 0, expectedRxlen);
    status = MODBUS_Transmit(sendBuffer, 8, 100);

    if (status != MODBUS_OK) return MODBUS_ERROR;
    return MODBUS_Receive(ReadBuffer, expectedRxlen, 100);
}