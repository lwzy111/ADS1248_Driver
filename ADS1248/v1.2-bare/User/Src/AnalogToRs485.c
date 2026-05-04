//
// Created by emmqa on 2026/2/8.
//

#include "../Inc/AnalogToRs485.h"
float AnalogToRs485_Voltage[8];

/**
 * @brief 模拟量转RS485模块初始化函数
 * @return ANALOGTORS485_StatusTypedef
 */
ANALOGTORS485_StatusTypedef AnalogToRs485_Init(void)
{
    HAL_StatusTypeDef status;

    uint16_t sendBuffer[3];
    sendBuffer[0] = 0x0000;
    sendBuffer[1] = 0x0001;
    sendBuffer[2] = 0x0001;

    status = MODBUS_WriteMultipleHoldRegs(ANALOGTORS485_SLAVE_ADDRESS,
             ANALOGTORS485_HOLD_Upload_Interval, 3, sendBuffer);
    if (status != MODBUS_OK) return ANALOGTORS485_ERROR;
    HAL_Delay(100); // 给 Flash 写入留一点喘息时间

    status = MODBUS_SetSingleHoldReg(ANALOGTORS485_SLAVE_ADDRESS,
             ANALOGTORS485_HOLD_Data_Format, 0x03);
    if (status != MODBUS_OK) return ANALOGTORS485_ERROR;
    HAL_Delay(100); // 给 Flash 写入留一点喘息时间

    status = MODBUS_SetSingleHoldReg(ANALOGTORS485_SLAVE_ADDRESS,
             ANALOGTORS485_HOLD_Parity_Bit_Sel, 0x00);
    if (status != MODBUS_OK) return ANALOGTORS485_ERROR;
    HAL_Delay(100); // 给 Flash 写入留一点喘息时间

    HAL_Delay(500); // 给模块留出处理和保存时间
    MODBUS_Set_RE();
    return ANALOGTORS485_OK;
}

/**
 * @brief 模拟量转RS485模块测量函数
 * @return ANALOGTORS485_StatusTypedef
 */
ANALOGTORS485_StatusTypedef AnalogToRs485_Measure(void)
{

    uint8_t ReadBuffer[21];
    if (MODBUS_ReadInputReg(ANALOGTORS485_SLAVE_ADDRESS,
    ANALOGTORS485_INPUT_REG_CH1, ReadBuffer, 8) == MODBUS_OK)
    {
        // 校验 CRC 并解析数据 (省略校验过程)
        if (MODBUS_CheckCRC16(ReadBuffer, 21) == MODBUS_OK)
        {
            // 发送到上位机调试
            // HAL_UART_Transmit(&huart2, ReadBuffer, 21, 100);
            memset(AnalogToRs485_Voltage, 0, sizeof(AnalogToRs485_Voltage));
            for (uint8_t i = 0; i < 8; i++)
            {
                uint16_t raw = (uint16_t)(ReadBuffer[i * 2 + 3] << 8 | ReadBuffer[i * 2 + 4]);
                AnalogToRs485_Voltage[i] = AnalogToRs485_ToFloat(raw, 4);
                // UART_Send_Float(&huart2, AnalogToRs485_Voltage[i]);
            }
        }
        else return ANALOGTORS485_CRCERROR;
    }
    else
    {
        // 如果接收超时，可以清理一下串口状态，防止下次卡死
        HAL_UART_AbortReceive(&MODBUS_UART);
        return ANALOGTORS485_ERROR;
    }
    // HAL_Delay(100);
    return ANALOGTORS485_OK;
}

/**
 * @brief 发送数据到CAN
 * @return ANALOGTORS485_StatusTypedef
 */
ANALOGTORS485_StatusTypedef AnalogToRs485_SendToCan(void)
{
    if (CAN_Send_Float(ANALOGTORS485_CAN_BEGINADDRESS, AnalogToRs485_Voltage,
                       ANALOGTORS485_CAN_DATALEN, 0.0f, 12.0f) != HAL_OK)
    return ANALOGTORS485_ERROR;
    else return ANALOGTORS485_OK;
}

/**
 * @brief 将 Modbus 原始数据转换为浮点数 (电压/电流)
 * @param rawValue 串口读到的 16 位寄存器值 (如 ReadBuffer[3]<<8 | ReadBuffer[4])
 * @param decimalPos 小数点位置 (对应寄存器 003AH 的设置，如 3 表示 3 位小数点)
 * @return float 转换后的实际物理数值
 */
float AnalogToRs485_ToFloat(uint16_t rawValue, uint8_t decimalPos)
{
    float val;

    // 1. 先按小数点位数还原为基本电压值 (单位: V)
    // 例如：1234 -> 1.234V (当 decimalPos = 3)
    float divider = 1.0f;
    for(int i = 0; i < decimalPos; i++) {
        divider *= 10.0f;
    }
    val = (float)rawValue / divider;

    return val;
}