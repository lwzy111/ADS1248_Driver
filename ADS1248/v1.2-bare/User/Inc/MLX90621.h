//
// Created by emmqa on 2025/12/19.
//

#ifndef SENSOR_MODULE_MLX90621_H
#define SENSOR_MODULE_MLX90621_H

#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include "../../Core/Inc/i2c.h"
#include "../../Core/Inc/can.h"

#define CAL_ACOMMON_L 0xD0
#define CAL_ACOMMON_H 0xD1
#define CAL_ACP_L 0xD3
#define CAL_ACP_H 0xD4
#define CAL_BCP 0xD5
#define CAL_alphaCP_L 0xD6
#define CAL_alphaCP_H 0xD7
#define CAL_TGC 0xD8
#define CAL_AI_SCALE 0xD9
#define CAL_BI_SCALE 0xD9

/** TA计算 **/

/*基准电压值*/
#define VTH_L 0xDA
#define VTH_H 0xDB
/*一阶线性系数*/
#define KT1_L 0xDC
#define KT1_H 0xDD
/*二阶线性系数*/
#define KT2_L 0xDE
#define KT2_H 0xDF
/*补偿单元数值*/
#define KT_SCALE 0xD2

//Common sensitivity coefficients
#define CAL_A0_L 0xE0
#define CAL_A0_H 0xE1
#define CAL_A0_SCALE 0xE2
#define CAL_DELTA_A_SCALE 0xE3
#define CAL_EMIS_L 0xE4
#define CAL_EMIS_H 0xE5
#define CAL_KSTA_L 0xE6
#define CAL_KSTA_H 0xE7

//Config register = 0xF5-F6
#define OSC_TRIM_VALUE 0xF7

//Bits within configuration register 0x92
#define POR_TEST 10

#define KS4_EE 0xC4
#define Ks_scale 0xC0

// 定义超时时间（以毫秒为单位）
#define TIMEOUT 100 // 100ms

uint8_t MLX90621_Init(void);
uint8_t MLX90621_Measure(void);
/**
 * @brief 发送数据到CAN
 * @return 0/1
 */
uint8_t MLX90621_SendToCan(void);

extern uint8_t eepromData[256];
extern float minTemp, maxTemp;
extern float temperatures[64];
extern int16_t irData[64];
extern float Tambient;
extern int refrate;
extern float tem_average;
#endif //SENSOR_MODULE_MLX90621_H