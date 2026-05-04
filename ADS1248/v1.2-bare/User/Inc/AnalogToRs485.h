//
// Created by emmqa on 2026/2/8.
//

#ifndef SENSOR_MODULE_BARE_ANALOGTORS485_H
#define SENSOR_MODULE_BARE_ANALOGTORS485_H

#include "modbus.h"
#include "../../Core/Inc/can.h"


#define ANALOGTORS485_SLAVE_ADDRESS                0x01    //从机地址
#define ANALOGTORS485_CAN_BEGINADDRESS             0x100   //CAN数据帧起始ID
#define ANALOGTORS485_CAN_DATALEN                  2       //CAN数据帧数量
#define ANALOGTORS485_CAN_STATUSADDRESS            0x100   //CAN状态帧ID

#define ANALOGTORS485_INPUT_REG_CH1                0x0000  /* 通道1 */
#define ANALOGTORS485_INPUT_REG_CH2                0x0001  /* 通道2 */
#define ANALOGTORS485_INPUT_REG_CH3                0x0002  /* 通道3 */
#define ANALOGTORS485_INPUT_REG_CH4                0x0003  /* 通道4 */
#define ANALOGTORS485_INPUT_REG_CH5                0x0004  /* 通道5 */
#define ANALOGTORS485_INPUT_REG_CH6                0x0005  /* 通道6 */
#define ANALOGTORS485_INPUT_REG_CH7                0x0006  /* 通道7 */
#define ANALOGTORS485_INPUT_REG_CH8                0x0007  /* 通道8 */

/* 保持寄存器地址 */
#define ANALOGTORS485_HOLD_Upload_Interval         0x0031    //上传间隔_设置
#define ANALOGTORS485_HOLD_Slave_ID                0x0032    //设备地址
#define ANALOGTORS485_HOLD_Baud_Rate_Sel           0x0033    //波特率设置
#define ANALOGTORS485_HOLD_Data_Format             0x003A    //数据解析方式设置
#define ANALOGTORS485_HOLD_Parity_Bit_Sel          0x003D    //奇偶校验设置

typedef enum
{
    ANALOGTORS485_OK = 0,
    ANALOGTORS485_ERROR,
    ANALOGTORS485_CRCERROR
} ANALOGTORS485_StatusTypedef;

extern float AnalogToRs485_Voltage[8];



/**
 * @brief 模拟量转RS485模块初始化函数
 * @return ANALOGTORS485_StatusTypedef
 */
ANALOGTORS485_StatusTypedef AnalogToRs485_Init(void);

/**
 * @brief 发送数据到CAN
 * @return ANALOGTORS485_StatusTypedef
 */
ANALOGTORS485_StatusTypedef AnalogToRs485_SendToCan(void);

/**
 * @brief 模拟量转RS485模块测量函数
 * @return ANALOGTORS485_StatusTypedef
 */
ANALOGTORS485_StatusTypedef AnalogToRs485_Measure(void);

/**
 * @brief 将 ANALOGTORS485 原始数据转换为浮点数 (电压/电流)
 * @param rawValue 串口读到的 16 位寄存器值 (如 ReadBuffer[3]<<8 | ReadBuffer[4])
 * @param decimalPos 小数点位置 (对应寄存器 003AH 的设置，如 3 表示 3 位小数点)
 * @return float 转换后的实际物理数值
 */
float AnalogToRs485_ToFloat(uint16_t rawValue, uint8_t decimalPos);

#endif //SENSOR_MODULE_BARE_ANALOGTORS485_H