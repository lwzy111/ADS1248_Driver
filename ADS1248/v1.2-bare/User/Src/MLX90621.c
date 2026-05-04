//
// Created by emmqa on 2025/12/19.
#include "main.h"
#include "usart.h"
#include "../Inc/MLX90621.h"

#include <string.h>
#define EEPROM_ADDRESS 0x50 << 1
#define RAM_ADDRESS 0x60 << 1
char message1[50] = "aaa";
int refrate;                //刷新率
float temperatures[64];     //存储温度矩阵64个温度点
float Tambient;             //传感器环境温度
uint8_t eepromData[256];    //存储eeprom数据
int16_t irData[64];         //存储ir数据
uint16_t config;            //配置寄存器值
uint8_t loopCount = 0; //Used in main loop
float v_ir_off_comp, ksta, v_ir_tgc_comp, v_ir_comp, alpha_comp;
float tak4, resolution_comp;
int16_t a_common, a_i_scale, b_i_scale, k_t1_scale, k_t2_scale, resolution;
float k_t1, k_t2, emissivity, tgc, alpha_cp, a_cp, b_cp, v_th;
uint16_t ptat;
int16_t cpix;
float a_ij, b_ij, alpha_ij;
float minTemp, maxTemp;

uint32_t timeout_start, current_time;
//计算
int16_t twos_16(uint8_t highByte, uint8_t lowByte){
    uint16_t combined_word = (highByte << 8) | lowByte;
    if (combined_word > 32767)
        return (int16_t) (combined_word - 65536);
    return (int16_t) combined_word;
}

int8_t twos_8(uint8_t byte) {
    if (byte > 127)
        return (int8_t) byte - 256;
    return (int8_t) byte;
}

uint16_t unsigned_16(uint8_t highByte, uint8_t lowByte){
    return (highByte << 8) | lowByte;
}

/*
 *函数:   读取eeprom数据
 *参数:   无
 *返回值:  0/1
 */
uint8_t MLX90621_ReadEEPROM(void)
{
    uint8_t Command[1];
    Command[0] = 0x00;
    timeout_start = HAL_GetTick();
    // 先发送命令，使用重复起始条件，不发送停止条件
    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, EEPROM_ADDRESS, Command, 1, I2C_FIRST_FRAME) != HAL_OK)
    {
        return 0;
    }

    // 等待传输完成
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }
    timeout_start = HAL_GetTick();
    // 然后接收数据，使用重复起始条件，并以停止条件结束
    if (HAL_I2C_Master_Seq_Receive_IT(&hi2c1, EEPROM_ADDRESS, eepromData, 256, I2C_LAST_FRAME) != HAL_OK) return 0;
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }

    return 1;
}
/*
 *函数:   配置振荡寄存器
 *参数:   无
 *返回值:  0/1
 */
uint8_t MLX90621_WriteTrimming(void)
{
    uint8_t trimData[5];
    trimData[0] = 0x04;
    int16_t temp = (int16_t)eepromData[0xF7] - 0xAA;
    trimData[1] = (uint8_t)temp;
    trimData[2] = eepromData[0xF7];
    trimData[3] = 0x56;
    trimData[4] = (unsigned char)0x00;

    timeout_start = HAL_GetTick();
    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, trimData, 5, I2C_LAST_FRAME) != HAL_OK) return 0;
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }
    return 1;
}

/*
 *函数:   配置配置寄存器
 *参数:   无
 *返回值:  0/1
 */
uint8_t setConfiguration(int refrate) {
    uint8_t Hz_LSB;
    switch (refrate) {
    case 0:
        Hz_LSB = 0x3F;      //	Hz_LSB = 0b00111111;
        break;
    case 1:
        Hz_LSB = 0x3E;      //	Hz_LSB = 0b00111110;
        break;
    case 2:
        Hz_LSB = 0x3D;      //	Hz_LSB = 0b00111101;
        break;
    case 4:
        Hz_LSB = 0x3C;      //	Hz_LSB = 0b00111100;
        break;
    case 8:
        Hz_LSB = 0x3B;      //	Hz_LSB = 0b00111011;
        break;
    case 16:
        Hz_LSB = 0x3A;      //	Hz_LSB = 0b00111010;
        break;
    case 32:
        Hz_LSB = 0x39;      //	Hz_LSB = 0b00111001;
        break;
    default:
        Hz_LSB = 0x3E;
    }
    uint8_t defaultConfig_H = 0x46;  //kmoto: See data sheet p.11 and 25
    uint8_t configData[5];
    int16_t temp;
    configData[0] = 0x03;
    temp = (int16_t)Hz_LSB - 0x55;
    configData[1] = (uint8_t)temp;
    configData[2] = Hz_LSB;
    temp = (int16_t)defaultConfig_H - 0x55;
    configData[3] = (uint8_t)defaultConfig_H - 0x55;
    //	configData[3] = (uint8_t)((int16_t)defaultConfig_H - 0x55);
    configData[4] = defaultConfig_H;

    timeout_start = HAL_GetTick();
    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, configData, 5, I2C_LAST_FRAME) != HAL_OK) return 0;
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }
    return 1;
}

/*
 *函数:   读配置寄存器
 *参数:   无
 *返回值:  0/1
 */
uint8_t readConfig(void) {
    uint8_t Command[4],ReadData[2];
    Command[0] = 0x02;
    Command[1] = 0x92;
    Command[2] = 0x00;
    Command[3] = 0x01;
    // if (HAL_I2C_Master_Transmit(&hi2c1, RAM_ADDRESS, Command, 4, 1000) != HAL_OK) return 0;
    // if (!HAL_I2C_Master_Receive(&hi2c1, RAM_ADDRESS, ReadData, 2, 1000)) return 0;
    // uint8_t configLow = ReadData[0];
    // uint8_t configHigh = ReadData[1];
    // config = ((uint16_t) (configHigh << 8) | configLow);
    // return 1;
    // while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
    //     // 可加入超时机制
    // }
    // if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, Command, 4, I2C_FIRST_FRAME) != HAL_OK) return 0;
    // while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
    //     // 可加入超时机制
    // }
    // HAL_Delay(5);
    // if (HAL_I2C_Master_Receive(&hi2c1, RAM_ADDRESS, ReadData, 2, 1000) != HAL_OK) return 0;
    // 先发送命令，使用重复起始条件，不发送停止条件
    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, Command, 4, I2C_FIRST_FRAME) != HAL_OK) {
        return 0;
    }

    // 等待传输完成
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }

    // 然后接收数据，使用重复起始条件，并以停止条件结束
    if (HAL_I2C_Master_Seq_Receive_IT(&hi2c1, RAM_ADDRESS, ReadData, 2, I2C_LAST_FRAME) != HAL_OK) {
        return 0;
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }
    uint8_t configLow = ReadData[0];
    uint8_t configHigh = ReadData[1];
    config = ((uint16_t) (configHigh << 8) | configLow);
    return 1;
}

/*
 *函数:   判断pro标志位
 *参数:   无
 *返回值:  0/1
 */
uint8_t checkConfig(void) {
    uint8_t check = !((config & 0x0400) >> 10);
    return check;
}

/**
 * @brief 读”与绝对温度成正比“传感器,用于计算传感器自身温度
 * @return
 */
uint8_t readPTAT(void) {
    uint8_t Command[4],ReadData[2];
    Command[0] = 0x02;
    Command[1] = 0x40;
    Command[2] = 0x00;
    Command[3] = 0x01;

    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, Command, 4, I2C_FIRST_FRAME) != HAL_OK) {
        return 0;
    }

    //等待传输完成
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }
    // 然后接收数据，使用重复起始条件，并以停止条件结束
    if (HAL_I2C_Master_Seq_Receive_IT(&hi2c1, RAM_ADDRESS, ReadData, 2, I2C_LAST_FRAME) != HAL_OK) {
        return 0;
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }
    uint8_t ptatLow = ReadData[0];
    uint8_t ptatHigh = ReadData[1];
    ptat = ((uint16_t)(ptatHigh << 8) | ptatLow);
    return 1;
}

/*
 *函数:   IR传感器结果
 *参数:   无
 *返回值:  0/1
 */
int8_t readIR(void) {
    uint8_t Command[4],ReadData[128];
    Command[0] = 0x02;
    Command[1] = 0x00;
    Command[2] = 0x01;
    Command[3] = 0x40;
    // 先发送命令，使用重复起始条件，不发送停止条件
    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, Command, 4, I2C_FIRST_FRAME) != HAL_OK) {
        return 0;
    }
    // 等待传输完成
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }

    // 然后接收数据，使用重复起始条件，并以停止条件结束
    if (HAL_I2C_Master_Seq_Receive_IT(&hi2c1, RAM_ADDRESS, ReadData, 128, I2C_LAST_FRAME) != HAL_OK) {
        return 0;
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }

    for(int i = 0; i < 128; i += 2) {
        uint8_t pixelDataLow = ReadData[i];
        uint8_t pixelDataHigh = ReadData[i + 1];
        irData[i/2] = twos_16(pixelDataHigh, pixelDataLow);
    }
    return 1;
}

/*
 *函数:   补偿像素结果
 *参数:   无
 *返回值:  0/1
 */
int8_t readCPIX(void) {
    uint8_t Command[4],ReadData[2];
    Command[0] = 0x02;
    Command[1] = 0x41;
    Command[2] = 0x00;
    Command[3] = 0x01;
    // 先发送命令，使用重复起始条件，不发送停止条件
    if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, RAM_ADDRESS, Command, 4, I2C_FIRST_FRAME) != HAL_OK) {
        return 0;
    }
    // 等待传输完成
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);
    }

    // 然后接收数据，使用重复起始条件，并以停止条件结束
    if (HAL_I2C_Master_Seq_Receive_IT(&hi2c1, RAM_ADDRESS, ReadData, 2, I2C_LAST_FRAME) != HAL_OK) {
        return 0;
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        // 可以加入超时机制
        current_time = HAL_GetTick();
        if ((current_time - timeout_start) >= 1000) return 0;
        HAL_Delay(1);

    }
    uint8_t cpixLow = ReadData[0];
    uint8_t cpixHigh = ReadData[1];
    cpix = twos_16(cpixHigh, cpixLow);
    return 1;
}

/*
 *函数:   计算
 *参数:   无
 *返回值:  0/1
 */
void preCalculateConstants(void) {
    /*分辨率补偿因子*/
    resolution_comp = pow(2.0, (3 - resolution));
    /*发射率系数*/
    emissivity = unsigned_16(eepromData[CAL_EMIS_H], eepromData[CAL_EMIS_L]) / 32768.0;
    a_common = twos_16(eepromData[CAL_ACOMMON_H], eepromData[CAL_ACOMMON_L]);
    a_i_scale = (int16_t)(eepromData[CAL_AI_SCALE] & 0xF0) >> 4;
    b_i_scale = (int16_t) eepromData[CAL_BI_SCALE] & 0x0F;

    alpha_cp = unsigned_16(eepromData[CAL_alphaCP_H], eepromData[CAL_alphaCP_L]) /
               (pow(2.0, eepromData[CAL_A0_SCALE]) * resolution_comp);
    a_cp = (float) twos_16(eepromData[CAL_ACP_H], eepromData[CAL_ACP_L]) / resolution_comp;
    b_cp = (float) twos_8(eepromData[CAL_BCP]) / (pow(2.0, (float)b_i_scale) * resolution_comp);
    tgc = (float) twos_8(eepromData[CAL_TGC]) / 32.0;

    /* 灵敏度随环境温度变化的补偿系数 */
    ksta = (float) twos_16(eepromData[CAL_KSTA_H], eepromData[CAL_KSTA_L]) / pow(2.0, 20.0);

    /****      计算TA的计算所需要的参数       ****/

    /*计算TA的一阶线性系数的缩放因子*/
    k_t1_scale = (int16_t) (eepromData[KT_SCALE] & 0xF0) >> 4;
    /*计算TA的二阶线性系数的缩放因子*/
    k_t2_scale = (int16_t) (eepromData[KT_SCALE] & 0x0F) + 10;
    /*计算基准电压值v_th*/
    v_th = (float) twos_16(eepromData[VTH_H], eepromData[VTH_L]);
    v_th = v_th / resolution_comp;
    /*计算TA的一阶线性系数*/
    k_t1 = (float) twos_16(eepromData[KT1_H], eepromData[KT1_L]);
    k_t1 /= (pow(2, k_t1_scale) * resolution_comp);
    /*计算TA的二阶线性系数*/
    k_t2 = (float) twos_16(eepromData[KT2_H], eepromData[KT2_L]);
    k_t2 /= (pow(2, k_t2_scale) * resolution_comp);
}

/**
 * @brief 计算PTAT(传感器自身温度，即绝对温度)
 */
void calculateTA(void) {
    Tambient = ((-k_t1 + sqrt(pow(k_t1, 2) - (4 * k_t2 * (v_th - (float) ptat))))
            / (2 * k_t2)) + 25.0;
}

/**
 * @brief 计算TO(目标温度)
 * @note 这是最核心的一步，基于 斯特藩-玻尔兹曼定律（Stefan-Boltzmann Law），即物体的辐射能量与温度的四次方成正比。
 */
void calculateTO(void) {
    float v_cp_off_comp = (float) cpix - (a_cp + b_cp * (Tambient - 25.0));
    tak4 = pow((float) Tambient + 273.15, 4.0);
    for (int i = 0; i < 64; i++) {
        // 一.计算经过寄生信号补偿后的红外像素原始信号v_ir_comp
        /* 偏移量offset */
        a_ij = ((float) a_common + eepromData[i] * pow(2.0, a_i_scale)) / resolution_comp;
        /* 斜率coefficient */
        b_ij = (float) twos_8(eepromData[0x40 + i]) / (pow(2.0, b_i_scale) * resolution_comp);
        /* 1.偏移值补偿（Offset Compensation）*/
        v_ir_off_comp = (float) irData[i] - (a_ij + b_ij * (Tambient - 25.0));
        /* 2.热梯度补偿（TGC）*/
        v_ir_tgc_comp = (float) v_ir_off_comp - tgc * v_cp_off_comp;
        /* 3.发生率补偿 (Emissivity compensation)*/
        v_ir_comp = v_ir_tgc_comp / emissivity;

        //二.计算经过补偿后的像素灵敏度系数

        /* 第 (i,j)个像素点的个别灵敏度系数alpha_ij */
        float alpha_ij = ((float) unsigned_16(eepromData[CAL_A0_H], eepromData[CAL_A0_L]) / pow(2.0, (float) eepromData[CAL_A0_SCALE]));
        alpha_ij += ((float) eepromData[0x80 + i] / pow(2.0, (float) eepromData[CAL_DELTA_A_SCALE]));
        alpha_ij = alpha_ij / resolution_comp;

        alpha_comp = (1 + ksta * (Tambient - 25.0)) * (alpha_ij - tgc * alpha_cp);

        //三.计算Ks4
        /*灵敏度补偿因子,BAB和BAD型的Ks4为0,可以简化*/
        //		float Ks4 = (float)twos_8(eepromData[KS4_EE]) / pow(2.0, (float)eepromData[Ks_scale] + 8);
        //		float Sx_ij = Ks4 * pow((pow(alpha_comp, 3) * v_ir_tgc_comp + pow(alpha_comp, 4) * tak4), 0.25);


        float temperature = pow((v_ir_comp / alpha_comp) + tak4, 1.0 / 4.0) - 274.15;
        //		float temperature = pow((v_ir_comp / (alpha_comp * (1 - Ks4 * 273.15) + Sx_ij)) + tak4, 1.0 / 4.0) - 274.15;
        temperatures[i] = temperature;
        if (i == 0)
        {
            minTemp = temperature;
            maxTemp = temperature;
        } else
        {
            if (minTemp > temperature) minTemp = temperature;
            if (maxTemp < temperature) maxTemp = temperature;
        }
    }
}

/*
 *函数:   初始化
 *参数:   无
 *返回值:  0/1
 */
uint8_t MLX90621_Init(void)
{
    HAL_Delay(5);
    if(!MLX90621_ReadEEPROM())  return 0;

    if(!MLX90621_WriteTrimming())  return 0;
    if(!setConfiguration(refrate))  return 0;
    if(!readConfig())  return 0;
    resolution = (config & 0x30) >> 4;
    preCalculateConstants();
    return 1;
}
/*
 *函数:   测量
 *参数:   无
 *返回值:  0/1
 */
uint8_t MLX90621_Measure(void) {
    if(!readConfig())  return 0;
    if (checkConfig()) {
        if(!MLX90621_ReadEEPROM())  return 0;
        if(!MLX90621_WriteTrimming())  return 0;
        if(!setConfiguration(refrate))  return 0;
    }
    if(!readPTAT())  return 0;
    if(!readIR())  return 0;
    if(!readCPIX())  return 0;
    calculateTA();
    calculateTO();
    // if(calculate_temps){
    // 	calculateTA();
    // 	readCPIX();
    // 	calculateTO();
    //		}
    return 1;
}

/**
 * @brief 发送数据到CAN
 * @return 0/1
 */
uint8_t MLX90621_SendToCan(void)
{
    if (CAN_Send_Float(0x90, temperatures,
                       16, -40.0f, 125.0f) != HAL_OK)
        return 0;
    else return 1;
}