//
// Created by emmqa on 2026/2/24.
//
#include "../Inc/RS485DigitalWeighingModule.h"

DWM_StatusTypedef DWM_ReadRealWeight(void)
{
    uint8_t response[9]; // 预期回复: 地址(1)+功能(1)+长度(1)+数据(4)+CRC(2) [cite: 194]
    if (MODBUS_ReadHoldReg(DWM_Slave_Address, DWM_Real_Time_Weight, response, 2) == MODBUS_OK)
    {
        if (MODBUS_CheckCRC16(response, 9) == MODBUS_OK)
        {
            uint32_t raw_data = (uint32_t)response[3] << 8  | (uint32_t)response[4] |
                            (uint32_t)response[5] << 24 | (uint32_t)response[6] << 16;
            DWM_RealWeight = (int32_t)raw_data;
            return DWM_OK;
        }
        return DWM_ERROR;
    }
    return DWM_ERROR;
}