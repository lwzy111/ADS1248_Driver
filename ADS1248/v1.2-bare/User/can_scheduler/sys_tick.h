/*
 * sys_tick.h
 *
 *  Created on: Mar 10, 2026
 *      Author: Geen
 */

#ifndef SYS_TICK_H_
#define SYS_TICK_H_

#include "stdint.h"

typedef enum
{
    CAN_90 = 0,
    CAN_91 = 1,
    CAN_92 = 2,
    CAN_93 = 3,
    CAN_94 = 4,
    CAN_95 = 5,
    CAN_96 = 6,
    CAN_97 = 7,
    CAN_98 = 8,
    CAN_99 = 9,
    CAN_9A = 10,
    CAN_9B = 11,
    CAN_9C = 12,
    CAN_9D = 13,
    CAN_9E = 14,
    CAN_9F = 15,
    CAN_102 = 16,
    TASK_MAX,
} Task_Enum;

typedef enum
{
    TASK_STATE_READY = 0,
    TASK_STATE_WAITING
} Task_State;

Task_State Get_Task_State(Task_Enum task);

void Delay_ms(Task_Enum task, uint32_t ms);


#endif /* SYS_TICK_H_ */
