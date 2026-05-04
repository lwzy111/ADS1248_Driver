
#ifndef SENSOR_MODULE_BARE_CAN_SCHEDULER_H
#define SENSOR_MODULE_BARE_CAN_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "can.h"
#include "tim.h"

/* CAN指针 */
#define CAN_SCHEDULER_HANDLE (&hcan)
/* CAN发送状态寄存器 */
#define CAN_SCHEDULER_TSR_REG (CAN_SCHEDULER_HANDLE->Instance->TSR)

/* 定时器中断周期 PERIOD = (Prescaler / HCLK * (Counter Period) * 1000)  */
#define Timer_INTERRUPT_PERIOD_ms 1

/* 各通道CAN帧的发送间隔 */
#define CAN_CH_HIGH_TX_INTERVAL_ms 10
#define CAN_CH_MEDIUM_TX_INTERVAL_ms 10
#define CAN_CH_LOW_TX_INTERVAL_ms 100

/*通道优先级定义*/
#define CAN_CH_HIGH     0
#define CAN_CH_MEDIUM   1
#define CAN_CH_LOW      2

/*CAN通道数*/
#define CAN_MAX_CHANNELS 3

/* CAN发送队列(环形缓冲区)长度 */
#define CAN_TX_QUEUE_SIZE 32

typedef enum {
    CAN_STATE_NORMAL = 0,
    CAN_STATE_ERROR_STOP,
    CAN_STATE_RECOVER_WAIT
} CAN_SchedulerState_t;


//CAN帧结构
typedef struct
{
    uint32_t StdId;     //标准ID(0x000 ~ 0x7FF)
    uint8_t Data[8];    //CAN数据(最多7字节)
    uint8_t Dlc;        //数据长度(0 ~ 8)
} CAN_Frame_t;

//CAN可配置的发送参数
typedef struct
{
    uint16_t Tx_interval_ms;    //发送间隔(1 ~ 65535ms)
    uint16_t node_id;           //节点ID基址
    uint8_t priority;           //发送优先级
    uint16_t tick_divider;      //分频计数器
    uint16_t tick_counter;        //当前计数
} CAN_TxConfig_t;

/* CAN环形数据缓冲区 */
typedef struct
{
    CAN_Frame_t Buffer[CAN_TX_QUEUE_SIZE];  //CAN帧存储数组
    volatile uint8_t ReadIndex;             //环形缓冲区读索引
    volatile uint8_t WriteIndex;            //环形缓冲区写索引
    volatile uint8_t count;                 //当前帧数(避免计算WriteIndex - ReadIndex)
} CAN_RingBuffer_t;

/* 初始化函数 */
void CAN_Scheduler_Init(void);
void CAN_Scheduler_StartTimer(TIM_HandleTypeDef *htim);

/* 帧操作 */
bool CAN_PushFrame(const CAN_Frame_t *frame, uint8_t priority);
void CAN_Scheduler_Process(void);   //主循环中调用，处理实际发送

/* 定时器中断回调(由HAL库调用) */
void CAN_Scheduler_TimerCallback(void); //在HAL_TIM_PeriodElapsedCallback中调用

#endif //SENSOR_MODULE_BARE_CAN_SCHEDULER_H