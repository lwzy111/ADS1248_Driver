/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  /* 配置过滤器：接收所有标准ID */
  CAN_FilterTypeDef sFilterConfig = {
    .FilterBank = 0,
    .FilterMode = CAN_FILTERMODE_IDMASK,
    .FilterScale = CAN_FILTERSCALE_32BIT,
    .FilterIdHigh = 0x0000,
    .FilterIdLow = 0x0000,
    .FilterMaskIdHigh = 0x0000,
    .FilterMaskIdLow = 0x0000,
    .FilterFIFOAssignment = CAN_RX_FIFO0,
    .FilterActivation = ENABLE,
    .SlaveStartFilterBank = 14
};
  HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);

  /* 启动CAN外设 */
  HAL_CAN_Start(&hcan);

  /* 启动CAN接收(使能中断) */
  // HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
 * @brief 发送CAN帧
 * @param id: 标志ID (0~0x7FF)
 * @param data: 数据指针
 * @param len: 数据长度(0~8)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CAN_Driver_Send(uint32_t id, uint8_t *data, uint8_t len)
{
  if (len > 8) return HAL_ERROR;

  // 如果 3 个邮箱都满了，HAL_CAN_GetTxMailboxesFreeLevel 会返回 0
  uint32_t timeout = 1000; // 防止死等，设置一个简单的超时计数
  while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
  {
    timeout--;
    if (timeout == 0) return HAL_BUSY; // 邮箱一直满，返回繁忙
    // 也可以加个微小的延迟：HAL_Delay(1);
  }
  CAN_TxHeaderTypeDef txHeader = {
    .StdId = id,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = len,
    .TransmitGlobalTime = DISABLE
  };

  uint32_t txMailbox;

  // 2. 尝试添加消息到邮箱
  HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan, &txHeader, data, &txMailbox);

  return status;
}

/**
  * @brief  CAN接收中断回调
  * @param  hcan: CAN句柄
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_Msg_t rxMsg;
  CAN_RxHeaderTypeDef rxHeader;

  // 从FIFO0读取（必须匹配中断源）
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxMsg.Data) == HAL_OK)
  {
    rxMsg.StdId = rxHeader.StdId;
    rxMsg.DLC = rxHeader.DLC;

  }
}

/**
 * @brief 浮点数量化为uint16_t（四舍五入）
 * @param x        输入浮点数
 * @param x_min    量程下限
 * @param x_max    量程上限
 * @return         量化后的uint16_t值
 */
uint16_t Float_To_Uint16(float x, float x_min, float x_max) {
  // 限幅保护
  if (x < x_min) x = x_min;
  else if (x > x_max) x = x_max;

  float span = x_max - x_min;
  float ratio = (x - x_min) / span;

  // 四舍五入到最接近的整数
  uint32_t temp = (uint32_t)(ratio * 65535.0f + 0.5f);

  return (uint16_t)temp;
}

/**
 * @brief 将uint16_t拆分为高低字节并打包到CAN数据缓冲区
 * @param value    要打包的uint16_t值
 * @param buf      CAN数据缓冲区
 * @param pos      起始位置（0-6）
 * @param big_endian true=大端序(MSB在前), false=小端序(LSB在前)
 */
void Pack_Uint16_To_CanData(uint16_t value, uint8_t *buf, uint8_t pos, bool big_endian) {
  if (big_endian) {
    // 大端序：高字节在前
    buf[pos]     = (value >> 8) & 0xFF;  // MSB
    buf[pos + 1] = value & 0xFF;         // LSB
  } else {
    // 小端序：低字节在前
    buf[pos]     = value & 0xFF;         // LSB
    buf[pos + 1] = (value >> 8) & 0xFF;  // MSB
  }
}

/**
 * @brief 从CAN数据缓冲区提取uint16_t
 * @param buf      CAN数据缓冲区
 * @param pos      起始位置（0-6）
 * @param big_endian true=大端序, false=小端序
 * @return         提取的uint16_t值
 */
uint16_t Extract_Uint16_From_CanData(uint8_t *buf, uint8_t pos, bool big_endian) {
  if (big_endian) {
    return (uint16_t)((buf[pos] << 8) | buf[pos + 1]);
  } else {
    return (uint16_t)(buf[pos] | (buf[pos + 1] << 8));
  }
}

/**
 * @brief uint16_t反量化为浮点数
 * @param u        输入的uint16_t值
 * @param x_min    原始量程下限
 * @param x_max    原始量程上限
 * @return         还原后的浮点数
 */
float Uint16_To_Float(uint16_t u, float x_min, float x_max) {
  float span = x_max - x_min;
  return x_min + (span * (float)u) / 65535.0f;
}

/**
 * @brief 发送浮点数
 * @param CAN_ID 起始ID
 * @param pData 浮点数组
 * @param len CAN帧长度(一帧4个浮点数)
 * @param min 量程下限
 * @param max 量程上限
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CAN_Send_Float(uint16_t CAN_ID, float *pData, uint8_t len, float min, float max)
{
  for (uint8_t i = 0; i < len; i++)
  {
    uint16_t add = CAN_ID + i;
    uint8_t can_data[8];
    for (uint8_t j = 0; j < 4; j++)
    {
      uint16_t temp = Float_To_Uint16(pData[i * 4 + j],min, max);
      Pack_Uint16_To_CanData(temp, can_data, j * 2, true);
    }

    HAL_StatusTypeDef status;
    uint8_t retry = 0;
    do
    {
      status = CAN_Driver_Send(add, can_data, 8);
      if (status != HAL_OK)
      {
        // 如果检测到离线错误 (Bus-off) 或 严重的被动错误
        if (HAL_CAN_GetError(&hcan) & HAL_CAN_ERROR_BOF) {
          HAL_CAN_Stop(&hcan);  // 停止
          HAL_CAN_Start(&hcan); // 重启，这会清除错误标志位并重新激活控制器
        }
        retry++;
        HAL_Delay(2);   // 给 CAN 总线和调度器留出喘息时间
      }
    } while (status != HAL_OK && retry < 5);
    if (retry == 5) return HAL_ERROR;
  }
  return HAL_OK;
}
/* USER CODE END 1 */
