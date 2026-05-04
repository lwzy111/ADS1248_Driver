/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
  /* CAN消息结构体 */
  typedef struct {
    uint32_t StdId;     // 标准ID
    uint8_t DLC;        // 数据长度
    uint8_t Data[8];    // 数据
  } CAN_Msg_t;

  /**
 * @brief 发送CAN帧
 * @param id: 标志ID (0~0x7FF)
 * @param data: 数据指针
 * @param len: 数据长度(0~8)
 * @return HAL_StatusTypeDef
 */
  HAL_StatusTypeDef CAN_Driver_Send(uint32_t id, uint8_t *data, uint8_t len);

  /**
 * @brief 浮点数量化为uint16_t（四舍五入）
 * @param x        输入浮点数
 * @param x_min    量程下限
 * @param x_max    量程上限
 * @return         量化后的uint16_t值
 */
  uint16_t Float_To_Uint16(float x, float x_min, float x_max);

  /**
* @brief 将uint16_t拆分为高低字节并打包到CAN数据缓冲区
* @param value    要打包的uint16_t值
* @param buf      CAN数据缓冲区
* @param pos      起始位置（0-6）
* @param big_endian true=大端序(MSB在前), false=小端序(LSB在前)
*/
  void Pack_Uint16_To_CanData(uint16_t value, uint8_t *buf, uint8_t pos, bool big_endian);

  /**
* @brief 从CAN数据缓冲区提取uint16_t
* @param buf      CAN数据缓冲区
* @param pos      起始位置（0-6）
* @param big_endian true=大端序, false=小端序
* @return         提取的uint16_t值
*/
  uint16_t Extract_Uint16_From_CanData(uint8_t *buf, uint8_t pos, bool big_endian);

  /**
* @brief uint16_t反量化为浮点数
* @param u        输入的uint16_t值
* @param x_min    原始量程下限
* @param x_max    原始量程上限
* @return         还原后的浮点数
*/
  float Uint16_To_Float(uint16_t u, float x_min, float x_max);

  /**
 * @brief 发送浮点数
 * @param CAN_ID 起始ID
 * @param pData 浮点数组
 * @param len CAN帧长度(一帧4个浮点数)
 * @param min 量程下限
 * @param max 量程上限
 * @return HAL_StatusTypeDef
 */
  HAL_StatusTypeDef CAN_Send_Float(uint16_t CAN_ID, float *pData, uint8_t len, float min, float max);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

