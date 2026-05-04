//
// Created by emmqa on 2026/1/30.
//

#include <sys/types.h>
#include "ADS1248.h"
#include "usart.h"

/**
 * @brief 发送单字节命令
 * @param GPIOx: 对应芯片的 CS 所在端口
 * @param GPIO_Pin: 对应芯片的 CS 引脚号
 * @param Command: 指令码
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SendCommand(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t Command)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    /* 选中指定芯片 */
    if (HAL_SPI_Transmit(&hspi1, &Command,1,100) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        return HAL_ERROR;
    }
    /* 释放指定芯片 */
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    return HAL_OK;
}

/**
 * @brief ADS1248写寄存器
 * @param GPIOx: 对应芯片的 CS 所在端口
 * @param GPIO_Pin: 对应芯片的 CS 引脚号
 * @param RegAddr 起始寄存器地址
 * @param RegNum 地址数
 * @param pData 数据
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_WriteRegs(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t RegAddr, uint8_t RegNum, uint8_t* pData)
{
    if (RegNum == 0 || pData == NULL) return HAL_ERROR;

    uint8_t Header[2];
    Header[0] = ADS1248_WREG | (RegAddr & 0x0F);    //第1字节: 指令+地址
    Header[1] = (RegNum - 1) & 0x0F;                //第2字节: 数量-1

    /* 选中指定芯片 */
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    if (HAL_SPI_Transmit(&hspi1, Header, 2, 100) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        return HAL_ERROR;
    }
    if (HAL_SPI_Transmit(&hspi1, pData, RegNum, 100) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        return HAL_ERROR;
    }
    /* 释放指定芯片 */
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    return HAL_OK;
}

/**
 *@brief 写ADS1248寄存器
 * @param GPIOx 对应芯片的 CS 所在端口
 * @param GPIO_Pin 对应芯片的 CS 引脚号
 * @param RegAddr 起始寄存器地址
 * @param RegNum 要读取的寄存器数量
 * @param pBuffer 放读取结果的缓冲区
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_ReadRegs(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t RegAddr, uint8_t RegNum, uint8_t* pBuffer)
{
    if (RegNum == 0 || pBuffer == NULL) return HAL_ERROR;

    uint8_t Header[2];
    Header[0] = ADS1248_RREG | (RegAddr & 0x0F);    //第1字节: 指令+地址
    Header[1] = (RegNum - 1) & 0x0F;                //第2字节: 数量-1

    /* 选中指定芯片 */
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);

    if (HAL_SPI_Transmit(&hspi1, Header, 2, 100) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        return HAL_ERROR;
    }
    if (HAL_SPI_Receive(&hspi1, pBuffer, RegNum, 100) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        return HAL_ERROR;
    }
    /* 释放指定芯片 */
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    return HAL_OK;
}

/**
 * @brief 配置多路复用器控制寄存器0
 * @param GPIOx: 对应芯片的 CS 所在端口
 * @param GPIO_Pin: 对应芯片的 CS 引脚号
 * @param Mode: 4种差分对组合
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetMUX0(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t Mode)
{
    if (Mode > 3) return HAL_ERROR;
    uint8_t Command;
    switch (Mode)
    {
        case 0:
            Command = 0b00000001; //AIN0正,AIN1负
            break;
        case 1:
            Command = 0b00010011; //AIN2正,AIN3负
            break;
        case 2:
            Command = 0b00100101; //AIN4正,AIN5负
            break;
        case 3:
            Command = 0b00110111; //AIN6正,AIN7负
            break;
        default:
            return HAL_ERROR;
    }
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_MUX0, 1, & Command);
}

/**
 *@brief 配置偏置电压寄存器
 *@param GPIOx: 对应芯片的 CS 所在端口
 *@param GPIO_Pin: 对应芯片的 CS 引脚号
 *@param PinNum: 偏置电压输入引脚编号 AIN0~AIN7
 *@return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetVBIAS(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t PinNum)
{
    if (PinNum > 7) return HAL_ERROR;
    uint8_t Command = 1 << PinNum;
    return  ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_VBIAS, 1, &Command);
}

/**
 * @brief  配置 ADS1248 MUX1 寄存器
 * @param  GPIOx: 对应芯片的 CS 所在端口
 * @param  GPIO_Pin: 对应芯片的 CS 引脚号
 * @param  vref_ctrl: 内部参考电压控制 (应预先左移 5 位)
 * @param  ref_select: 参考源选择控制 (应预先左移 3 位)
 * @param  cal_mode: 系统监控控制 (0-7)
 * @return HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef ADS1248_SetMUX1(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
                                  uint8_t vref_ctrl, uint8_t ref_select, uint8_t cal_mode)
{
    // 1. 安全掩码：确保传入的参数仅修改其对应的位域
    uint8_t command = 0x00;

    // 逻辑上：VREFCON[1:0] | REFSELT[1:0] | MUXCAL[2:0]
    command |= (vref_ctrl  & ADS1248_MUX1_VREFCON_MASK);
    command |= (ref_select & ADS1248_MUX1_REFSELT_MASK);
    command |= (cal_mode   & ADS1248_MUX1_MUXCAL_MASK);

    // 2. 调用底层写入函数
    // 注意：Bit 7 (CLKSTAT) 是只读的，写入 0 会被芯片忽略
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_MUX1, 1, &command);
}

/**
 * @brief  配置 ADS1248 SYS0 寄存器 (增益与速率)
 * @param  GPIOx: CS 端口
 * @param  GPIO_Pin: CS 引脚
 * @param  pga: 增益设置 (使用 ADS1248_PGA_x 宏)
 * @param  dr: 数据速率设置 (使用 ADS1248_DR_x 宏)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetSYS0(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t pga, uint8_t dr)
{
    // 根据手册 Figure 98，Bit 7 必须始终设置为 0
    // 组合数据：(0 & 0x80) | PGA位 | DR位
    uint8_t Command = (pga & ADS1248_SYS0_PGA_MASK) | (dr & ADS1248_SYS0_DR_MASK);

    // SYS0 寄存器地址通常为 0x03
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_SYS0, 1, &Command);
}

/**
 * @brief  写入 24 位 OFC 偏移校准寄存器
 * @param  GPIOx: CS 端口
 * @param  GPIO_Pin: CS 引脚
 * @param  offset_val: 24 位补码格式的偏移值
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetOFC(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int32_t offset_val)
{
    uint8_t data[3];
    // 分解为三个字节：OFC[7:0], OFC[15:8], OFC[23:16]
    data[0] = (uint8_t)(offset_val & 0xFF);         // OFC[7:0] (地址 04h)
    data[1] = (uint8_t)((offset_val >> 8) & 0xFF);  // OFC[15:8] (地址 05h)
    data[2] = (uint8_t)((offset_val >> 16) & 0xFF); // OFC[23:16] (地址 06h)

    // 从起始地址 04h 开始连续写入 3 个寄存器
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_OFC0, 3, data);
}

/**
 * @brief  写入 24 位 FSC 满量程校准寄存器
 * @param  GPIOx: CS 端口
 * @param  GPIO_Pin: CS 引脚
 * @param  fsc_val: 24位无符号校准值
 */
HAL_StatusTypeDef ADS1248_SetFSC(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t fsc_val)
{
    uint8_t data[3];
    // 寄存器地址依次为 07h, 08h, 09h
    data[0] = (uint8_t)(fsc_val & 0xFF);
    data[1] = (uint8_t)((fsc_val >> 8) & 0xFF);
    data[2] = (uint8_t)((fsc_val >> 16) & 0xFF);

    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_FSC0, 3, data);
}

/**
 * @brief 配置IDAC0寄存器
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @param drdy_mode: DOUT/DRDY 引脚功能设置
 * @param imag: IDAC 励磁电流大小
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetIDAC0(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t drdy_mode, uint8_t imag)
{
    uint8_t Command = 0x00;
    Command |= (drdy_mode & ADS1248_IDAC0_DRDY_MODE_MASK);
    Command |= (imag & ADS1248_IDAC0_IMAG_MASK);

    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_IDAC0, 1, &Command);
}

/**
 * @brief 配置IDAC1寄存器
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @param I1DIR: IDAC输出电流源1引脚选择
 * @param I2DIR: IDAC输出电流源2引脚选择
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetIDAC1(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t I1DIR, uint8_t I2DIR)
{
    uint8_t Command = 0x00;
    Command |= (I1DIR & ADS1248_IDAC1_I1DIR_MASK);
    Command |= (I2DIR & ADS1248_IDAC1_I2DIR_MASK);

    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_IDAC1, 1, &Command);
}

/**
 * @brief 配置GPIO配置寄存器
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @param IOCFG: 选择GPIO引脚
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetGPIOCFG(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t IOCFG)
{
    /*1为启动,0为关闭*/
    /*IOCFG[7] --> IOCFG[0] */
    /*AIN7 --> AIN2、REFN0、REFP0*/
    uint8_t Command = 0x00;
    Command |= IOCFG;
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_GPIOCFG, 1, &Command);
}

/**
 * @brief 配置GPIO方向寄存器
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @param IODIR
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetGPIODIR(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t IODIR)
{
    /*1为输入,0为输出*/
    /*IOCFG[7] --> IOCFG[0] */
    /*AIN7 --> AIN2、REFN0、REFP0*/
    uint8_t Command = 0x00;
    Command |= IODIR;
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_GPIODIR, 1, &Command);
}

/**
 * @brief  配置GPIO数据寄存器
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @param IODAT
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetGPIODAT(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t IODAT)
{
    /*1为高电平,0为低电平*/
    /*IOCFG[7] --> IOCFG[0] */
    /*AIN7 --> AIN2、REFN0、REFP0*/
    uint8_t Command = 0x00;
    Command |= IODAT;
    return ADS1248_WriteRegs(GPIOx, GPIO_Pin, ADS1248_GPIODAT, 1, &Command);
}

/**
 * @brief ADS1248初始化
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    uint8_t cmd;
    HAL_StatusTypeDef status = HAL_OK;

    //发送复位指令
    status |= ADS1248_SendCommand(GPIOx, GPIO_Pin, ADS1248_RESET);
    HAL_Delay(200);

    //停止连续读模式
    status |= ADS1248_SendCommand(GPIOx, GPIO_Pin, ADS1248_SDATAC);
    HAL_Delay(200);

    //配置MUX1
    status |= ADS1248_SetMUX1(GPIOx, GPIO_Pin, ADS1248_VREF_ON, ADS1248_REF_INT, ADS1248_MON_NORMAL);
    HAL_Delay(200);

    // 4. 配置 SYS0: PGA = 1 (10V分压后约2V), 速率 = 20SPS (抗噪性好)
    status |= ADS1248_SetSYS0(GPIOx, GPIO_Pin, ADS1248_PGA_1, ADS1248_DR_20SPS);
    HAL_Delay(200);

    // 5. 执行自偏移校准
    status |= ADS1248_SendCommand(GPIOx, GPIO_Pin, ADS1248_SELFOCAL);
    // 校准完成后，DRDY 会变低
    uint32_t cal_timeout = 200000;
    // HAL_Delay(2000); // 校准需要时间

    while(HAL_GPIO_ReadPin(ADS1248_DRDY_PORT, ADS1248_DRDY_PIN) == GPIO_PIN_SET && cal_timeout--);
    if (cal_timeout == 0)
    {
        return HAL_ERROR;
    }


    return status;
}

/**
 * @brief  读取并换算 10V 分压后的实际电压
 * @param  GPIOx: CS 端口
 * @param  GPIO_Pin: CS 引脚
 * @param  ratio: 外部分压比例 (例如测量10V分压到2V，ratio则为 5.0)
 * @return float: 返回实际测得的电压值 (单位: V)
 */
float ADS1248_Measure_Voltage(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, float ratio)
{

    uint8_t cmd = ADS1248_SYNC;
    ADS1248_SendCommand(GPIOx, GPIO_Pin, cmd);

    // 等待DRDY变低（带500ms超时）
    uint32_t timeout = HAL_GetTick();
    while(HAL_GPIO_ReadPin(ADS1248_DRDY_PORT, ADS1248_DRDY_PIN) == GPIO_PIN_SET) {
        if(HAL_GetTick() - timeout > 1000) {
            return 1.666f;  // 超时返回错误值
        }
    }
    // // 1. 等待数据准备就绪 (假设 DRDY 接在 MCU 的某个输入引脚)
    // uint32_t timeout = 50000;
    // while(HAL_GPIO_ReadPin(ADS1248_DRDY_PORT, ADS1248_DRDY_PIN) == GPIO_PIN_SET && timeout--) {
    //     // 等待 DRDY 变低
    // }
    // if(timeout == 0) return 0.0f; // 超时错误


    cmd = ADS1248_RDATA;
    uint8_t rx_buf[4];
    int32_t raw_data = 0;

    // 1. 选中芯片并发送读取指令
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    // HAL_Delay(1);

    // // 2. 接收 3 字节原始数据
    if (HAL_SPI_TransmitReceive(&hspi1, &cmd, rx_buf, 4, 1) != HAL_OK)
    {
        return 1.666f;
    }
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);


    // 4. 发送 RDATA 命令 (1字节)
    // cmd = ADS1248_RDATA;
    // if (HAL_SPI_Transmit(&hspi1, &cmd, 1, 100) != HAL_OK) {
    //     HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    //     return -999.0f;
    // }

    // // 5. 发送 3 个 NOP (0xFF) 读取 24位数据
    // uint8_t tx_dummy[3] = {0xFF, 0xFF, 0xFF};
    // uint8_t rx_data[3] = {0};
    // if (HAL_SPI_TransmitReceive(&hspi1, tx_dummy, rx_data, 3, 100) != HAL_OK) {
    //     HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    //     return -999.0f;
    // }
    // raw_data = ((int32_t)rx_data[0] << 16) | ((int32_t)rx_data[1] << 8) | (int32_t)rx_data[2];
    // 3. 24位补码转32位有符号整数
    // raw_data = (int32_t)((rx_buf[0] << 16) | (rx_buf[1] << 8) | rx_buf[2]);
    // if (raw_data & 0x800000) { // 如果符号位为 1
    //     raw_data |= 0xFF000000; // 符号位扩展
    // }

    raw_data = ((int32_t)rx_buf[1] << 16) | ((int32_t)rx_buf[2] << 8) | (int32_t)rx_buf[3];
    if (raw_data & 0x800000) raw_data |= 0xFF000000;

    // 4. 电压换算公式
    // V = (Raw / (2^23 - 1)) * (Vref / PGA) * ratio
    // ADS1248 内部参考通常为 2.048V
    float vref = 2.048;
    float pga = 1.0f;

    float voltage = ((float)raw_data / 8388607.0f) * (vref / pga) * ratio;

    return voltage;
}