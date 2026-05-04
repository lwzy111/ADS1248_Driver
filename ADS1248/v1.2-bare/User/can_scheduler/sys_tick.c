/*
 * sys_tick.c
 *
 *  Created on: Mar 10, 2026
 *      Author: Geen
 */

#include "sys_tick.h"
#include "stm32f1xx_hal.h"

/* 系统 tick 结构体 */
typedef struct 
{
    uint32_t target_time;
    Task_Enum task;
    uint8_t index;
} SysTick_Struct;

/* 任务调度配置 */
SysTick_Struct SysTick_Configs[TASK_MAX] =
{
    {0, CAN_90, 0},
    {0, CAN_91, 1},
    {0, CAN_92, 2},
    {0, CAN_93, 3},
    {0, CAN_94, 4},
    {0, CAN_95, 5},
    {0, CAN_96, 6},
    {0, CAN_97, 7},
    {0, CAN_98, 8},
    {0, CAN_99, 9},
    {0, CAN_9A, 10},
    {0, CAN_9B, 11},
    {0, CAN_9C, 12},
    {0, CAN_9D, 13},
    {0, CAN_9E, 14},
    {0, CAN_9F, 15},
    {0, CAN_102, 16},
};


Task_State Get_Task_State(Task_Enum task)
{
    SysTick_Struct* config = NULL;
    /* 验证参数 */
    for (int i = 0; i < TASK_MAX; i++)
    {
        if (SysTick_Configs[i].index== task)
        {
            config = &SysTick_Configs[i];
            break;
        }
    }
    if (config == NULL)
    {
        return TASK_STATE_WAITING; // 无效任务
    }

    uint32_t tick = HAL_GetTick();
    /* 判断任务状态 */
    if (tick >= config->target_time)
    {
        return TASK_STATE_READY;
    }
    else
    {
        return TASK_STATE_WAITING;
    }
}



void Delay_ms(Task_Enum task, uint32_t ms) 
{
    SysTick_Struct* config = NULL;
    /* 验证参数 */
    for (int i = 0; i < TASK_MAX; i++)
    {
        if (SysTick_Configs[i].task == task)
        {
            config = &SysTick_Configs[i];
            break;
        }
    }
    if (config == NULL)
    {
        return; // 无效任务
    }

    /* 更新目标时间 */
    config->target_time = HAL_GetTick() + ms;
}
