/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MLX90621.h"
#include "modbus.h"
#include "AnalogToRs485.h"
#include "ADS1248.h"
#include "can_scheduler.h"
#include "sys_tick.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  bool CAN_SubmitBatch(CAN_Frame_t *frames, uint8_t count, uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    uint8_t submitted = 0;

    while(submitted < count) {
      if(CAN_PushFrame(&frames[submitted], CAN_CH_HIGH)) {
        submitted++;
      } else {
        // 队列满，驱动发送释放空间
        CAN_Scheduler_Process();

        if((HAL_GetTick() - start) > timeout_ms) {
          return false;  // 超时
        }
      }
    }
    return true;
  }
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  uint8_t flag= 1;
  float voltage[4];
  uint8_t CAN_Buffer[8];
  CAN_Frame_t ADS1248_Frame;
  CAN_Frame_t MLX90621_Frame[16];
  CAN_Frame_t aaa;
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
  refrate = 1;
  HAL_Delay(200);
  ADS1248_Init(GPIOB, GPIO_PIN_1);
  // MLX90621_Init();
  // // 连续测量两次但不发送，确保传感器内部流水线充满正确的数据
  // for(uint8_t warmup = 0; warmup < 2; warmup++) {
  //   MLX90621_Measure();
  //   HAL_Delay(200);
  // }

  // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  CAN_Scheduler_Init();
  CAN_Scheduler_StartTimer(&htim4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    memset(voltage, 0, sizeof(voltage));
    memset(temperatures, 0, sizeof(temperatures));

    // if (MLX90621_Measure() != 1) {
    //   // 此时说明 I2C 通信已经挂了
    //   // 1. 彻底关闭 I2C 外设
    //   __HAL_I2C_DISABLE(&hi2c1);
    //   HAL_Delay(5);
    //
    //   // 2. 重新初始化 I2C 硬件寄存器
    //   HAL_I2C_DeInit(&hi2c1);
    //   HAL_I2C_Init(&hi2c1);
    //   MLX90621_Init();      // 重新初始化
    //   // 连续测量2次但不发送，确保传感器内部流水线充满正确的数据
    //   for(uint8_t warmup = 0; warmup < 2; warmup++) {
    //     MLX90621_Measure();
    //   }
    //   MLX90621_Measure();
    // }




    for (uint8_t i = 0; i < 4; i++)
    {
      ADS1248_SetMUX0(GPIOB, GPIO_PIN_1, i);
      //measure voltage by the SPI device
      voltage[i] = ADS1248_Measure_Voltage(GPIOB, GPIO_PIN_1, 1);


      // // transmit by UART
      // char buffer[20];
      // float voltage_temp;
      // if (voltage[i] < 0)
      // {
      //   HAL_UART_Transmit(&huart3, (uint8_t *)"-", 1, HAL_MAX_DELAY);
      //   voltage_temp = -voltage[i];
      // }
      // else voltage_temp = voltage[i];
      // int voltage_int = (int)(voltage_temp * 100000);
      // int whole = voltage_int / 100000;
      // int decimal = voltage_int % 100000;
      // memset(buffer, 0, sizeof(buffer));
      // snprintf(buffer, sizeof(buffer), "%d.%05d", whole, decimal);
      // HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
      // HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 3, HAL_MAX_DELAY);
    }




    memset(CAN_Buffer, 0, sizeof(CAN_Buffer));
    for (uint8_t j = 0; j < 4; j++)
    {
      if (voltage[j] != -999.0)
      {
        uint16_t temp = Float_To_Uint16(voltage[j],-2.048f, 2.048f);
        Pack_Uint16_To_CanData(temp, CAN_Buffer, j * 2, true);
      }
    }

    ADS1248_Frame.StdId = 0x102;
    memcpy(ADS1248_Frame.Data, CAN_Buffer, 8);  // ✅ 用memcpy拷贝数组内容
    ADS1248_Frame.Dlc = 8;



    // for (uint8_t i = 0; i < 16; i ++)
    // {
    //   for (uint8_t j = 0; j < 4; j++)
    //   {
    //     if (temperatures[4 * i + j] > 0)
    //     {
    //       flag = 1;
    //       uint16_t temp = Float_To_Uint16(temperatures[4 * i + j],-40.0f, 125.0f);
    //       Pack_Uint16_To_CanData(temp, CAN_Buffer, j * 2, true);
    //     }
    //     else
    //     {
    //       flag = 0;
    //     }
    //   }
    //
    //   MLX90621_Frame[i].StdId = 0x90 + i;
    //   memcpy(MLX90621_Frame[i].Data, CAN_Buffer, 8);  // ✅ 用memcpy拷贝数组内容
    //   MLX90621_Frame[i].Dlc = 8;
    // }
    //
    //
    // if (flag == 1)
    // {
    //   CAN_SubmitBatch(MLX90621_Frame, 16, 100);
    // }


    CAN_PushFrame(&ADS1248_Frame, CAN_CH_MEDIUM);

    // for (uint8_t i = 0; i < 16; i++)
    // {
    //   if (Get_Task_State(i) != TASK_STATE_READY)
    //   {
    //     break;
    //   }
    //   CAN_PushFrame(&MLX90621_Frame[i], CAN_CH_MEDIUM);
    //   Delay_ms(i, 500);
    // }

    CAN_Scheduler_Process();

    // CAN_Send_Float(0x102, voltage, 1, -2.048f, 2.048f);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
