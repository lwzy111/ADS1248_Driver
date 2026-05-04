#include "can_scheduler.h"

/* 调度器主结构 */
static struct {
    CAN_RingBuffer_t queues[CAN_MAX_CHANNELS];          // 3个通道的队列

    // 多邮箱管理：3个bit，每bit表示一个邮箱状态
    // bit0=邮箱0忙, bit1=邮箱1忙, bit2=邮箱2忙
    volatile uint8_t tx_mailbox_busy;                   // 3个邮箱的占用位图

    // 错误恢复状态机
    CAN_SchedulerState_t state;                         // 当前状态机状态
    uint32_t recovery_start_time;                       // 错误恢复开始时间戳

    TIM_HandleTypeDef *htim;                            //定时器句柄
    volatile uint8_t tx_trigger[CAN_MAX_CHANNELS];      //发送触发标志(中断置位，主循环查询)
} CAN_sched;

/* 各CAN发送通道的默认配置 */
static CAN_TxConfig_t default_configs [CAN_MAX_CHANNELS] = {
    {CAN_CH_HIGH_TX_INTERVAL_ms, 0x100, CAN_CH_HIGH,
        CAN_CH_HIGH_TX_INTERVAL_ms * Timer_INTERRUPT_PERIOD_ms, 0},   //100ms周期，分频100次
    {CAN_CH_MEDIUM_TX_INTERVAL_ms, 0x000, CAN_CH_MEDIUM,
        CAN_CH_MEDIUM_TX_INTERVAL_ms * Timer_INTERRUPT_PERIOD_ms, 0},
    {CAN_CH_LOW_TX_INTERVAL_ms, 0x300, CAN_CH_LOW,
        CAN_CH_LOW_TX_INTERVAL_ms * Timer_INTERRUPT_PERIOD_ms, 0}
};

/**
 * @brief CAN调度器初始化
 */
void CAN_Scheduler_Init(void)
{
    for (int i = 0; i < CAN_MAX_CHANNELS; i++)
    {
        CAN_sched.queues[i].WriteIndex = 0;
        CAN_sched.queues[i].ReadIndex = 0;
        CAN_sched.queues[i].count = 0;          // 帧计数归零
        CAN_sched.tx_trigger[i] = 0;            // 清除触发标志
    }
    CAN_sched.tx_mailbox_busy = 0;              // 所有邮箱空闲
    CAN_sched.state = CAN_STATE_NORMAL;         // 初始状态：正常
    CAN_sched.htim = NULL;                      // 定时器未绑定
}

/**
 * @brief 定时器启动
 * @param htim: 定时器指针
 */
void CAN_Scheduler_StartTimer(TIM_HandleTypeDef *htim) {
    CAN_sched.htim = htim;

    /* 配置定时器为1ms周期中断 */
    /* 假设APB1时钟72MHz，预分频72-1，计数1000-1 = 1ms周期 */

    //在MX里配置
    // __HAL_TIM_SET_AUTORELOAD(htim, 1000 - 1);                 // 自动重载值：计数到999后归零
    // __HAL_TIM_SET_PRESCALER(htim, 72 - 1);                    // 预分频：72分频

    HAL_TIM_Base_Start_IT(htim);        // 启动定时器中断
}

/**
 * @brief 定时器中断回调 - 在HAL_TIM_PeriodElapsedCallback中调用
 */
void CAN_Scheduler_TimerCallback(void) {
    /* 在中断中只做：软件分频计数 + 置位触发标志 */
    for(int i = 0; i < CAN_MAX_CHANNELS; i++) {
        default_configs[i].tick_counter++;

        // 达到分频值？
        if(default_configs[i].tick_counter >= default_configs[i].tick_divider) {
            default_configs[i].tick_counter = 0;                // 清零计数器
            CAN_sched.tx_trigger[i] = 1;                        // 置位触发标志（主循环将处理）
        }
    }
}

/**
 * @brief 将要发送的CAN帧存入环形缓冲区
 * @param frame CAN帧
 * @param priority 优先级
 * @return CAN_Sched_StatusTypeDef
 */
bool CAN_PushFrame(const CAN_Frame_t *frame, uint8_t priority)
{
    //优先级限幅，防止越界
    if (priority > 2) priority = 2;

    CAN_RingBuffer_t *q = &CAN_sched.queues[priority];        // 获取对应队列

    // 检查队列满（关键：避免覆盖未发送数据）
    if (q -> count >= CAN_TX_QUEUE_SIZE)    return false;     //队列满，丢弃帧（或阻塞等待）

    // 关中断保护（或改用LDREX/STREX原子操作）
    uint32_t primask = __get_PRIMASK();         // 保存当前中断状态
    __disable_irq();                            // 关闭中断（原子操作保护）

    //写入数据到WriteIndex位置
    q -> Buffer[q -> WriteIndex] = *frame;

    //head指针前进（环形：模运算）
    q -> WriteIndex = (q -> WriteIndex + 1) % CAN_TX_QUEUE_SIZE;

    // 计数增加（原子操作，需关中断保护）
    q->count++;

    __set_PRIMASK(primask);
    return true;
}

/**
 * @brief 将要发送的CAN帧从环形缓冲区移除
 * @param frame CAN帧
 * @param priority 优先级
 * @return CAN_Sched_StatusTypeDef
 */
static bool CAN_PopFrame(CAN_Frame_t *frame, uint8_t priority)
{
    CAN_RingBuffer_t *q = &CAN_sched.queues[priority];

    if (q -> count == 0) return false;    //队列空

    *frame = q -> Buffer[q -> ReadIndex];   //读取tail位置

    q -> ReadIndex = (q -> ReadIndex + 1) % CAN_TX_QUEUE_SIZE;  //ReadIndex前进

    q -> count--;

    return true;
}

/**
 * @brief 获取空闲邮箱数量
 * @return 空闲邮箱数量
 */
static inline uint8_t CAN_GetFreeMailboxCount(void) {
    return HAL_CAN_GetTxMailboxesFreeLevel(CAN_SCHEDULER_HANDLE);
}

/**
 * @brief 检查特定邮箱是否空闲
 * @param mailbox 特定邮箱号
 * @return bool(空闲)
 */
static inline bool CAN_IsMailboxFree(uint8_t mailbox) {
    return !(CAN_sched.tx_mailbox_busy & (1 << mailbox));
}

/**
 * @brief 标记邮箱状态
 * @param mailbox 特定邮箱号
 */
static inline void CAN_MarkMailboxBusy(uint8_t mailbox) {
    CAN_sched.tx_mailbox_busy |= (1 << mailbox);
}

/**
 * @brief 发送完成回调（CAN中断中调用）
 * @param hcan
 */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
    CAN_sched.tx_mailbox_busy &= ~(1 << 0);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
    CAN_sched.tx_mailbox_busy &= ~(1 << 1);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
    CAN_sched.tx_mailbox_busy &= ~(1 << 2);
}

/**
 * @brief 调度器主程序
 */
void CAN_Scheduler_Process(void)
{
    uint32_t now = HAL_GetTick();  // 获取系统tick（通常1ms递增）

    // ========== 1. 错误恢复状态机（无阻塞） ==========
    switch(CAN_sched.state) {
    case CAN_STATE_NORMAL:
        if(HAL_CAN_GetState(CAN_SCHEDULER_HANDLE) == HAL_CAN_STATE_ERROR) {
            HAL_CAN_Stop(CAN_SCHEDULER_HANDLE);                                // 停止CAN控制器
            CAN_sched.state = CAN_STATE_ERROR_STOP;
            CAN_sched.recovery_start_time = now;                // 记录停止时间
        }
        break;

    case CAN_STATE_ERROR_STOP:
        // 非阻塞等待10ms
        if((now - CAN_sched.recovery_start_time) >= 10) {
            HAL_CAN_Start(CAN_SCHEDULER_HANDLE);                                // 重新启动CAN
            CAN_sched.state = CAN_STATE_NORMAL;
        }
        return;  // 恢复期间不发送

    default:
        CAN_sched.state = CAN_STATE_NORMAL;
        break;
    }

    // ========== 2. 更新邮箱状态（硬件查询作为备份） ==========
    // 如果中断丢失，通过硬件状态修正
    uint32_t tsr = CAN_SCHEDULER_TSR_REG;              // 读发送状态寄存器
    if(tsr & CAN_TSR_RQCP0) {                       // 邮箱0请求完成?
        CAN_SCHEDULER_TSR_REG = CAN_TSR_RQCP0;         // 写1清零: TSR = RQCP0
        CAN_sched.tx_mailbox_busy &= ~(1 << 0);     // 清除软件标记（备份机制）
    }
    if(tsr & CAN_TSR_RQCP1) {
        CAN_SCHEDULER_TSR_REG = CAN_TSR_RQCP1;
        CAN_sched.tx_mailbox_busy &= ~(1 << 1);
    }
    if(tsr & CAN_TSR_RQCP2) {
        CAN_SCHEDULER_TSR_REG = CAN_TSR_RQCP2;
        CAN_sched.tx_mailbox_busy &= ~(1 << 2);
    }

    // ========== 3. 批量发送（利用3个邮箱并行） ==========
    uint8_t free_mboxes = CAN_GetFreeMailboxCount();
    if(free_mboxes == 0) return;  // 全忙，下次再说

    // 按优先级尝试填充所有空闲邮箱
    for(int p = 0; p < CAN_MAX_CHANNELS && free_mboxes > 0; p++) {

        // 检查定时器触发标志
        if(CAN_sched.tx_trigger[p] == 0) continue;  // 未到发送时刻，跳过

        CAN_sched.tx_trigger[p] = 0;  // 立即清除标志！

        CAN_RingBuffer_t *q = &CAN_sched.queues[p];
        if(q->count == 0) continue;                // 队列为空，跳过

        // 关中断保护队列操作
        uint32_t primask = __get_PRIMASK();        // 进入临界区前保存当前状态
        __disable_irq();                           // 禁用中断（进入临界区）

        // 尝试发送（可能多个帧）
        while(q->count > 0 && free_mboxes > 0) {
            CAN_Frame_t frame;
            CAN_PopFrame(&frame, p);  // 出队一帧

            CAN_TxHeaderTypeDef header = {
                .StdId = frame.StdId,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .DLC = frame.Dlc,
                .TransmitGlobalTime = DISABLE       // 不发送时间戳
            };

            uint32_t mailbox;
            HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(CAN_SCHEDULER_HANDLE, &header, frame.Data, &mailbox);

            if(status == HAL_OK) {
                CAN_MarkMailboxBusy(mailbox);
                free_mboxes--;
            } else {
                // 发送失败，帧已丢失（应放回队列或报错）
                break;
            }
        }

        __set_PRIMASK(primask);         // 恢复之前的状态（而不是简单地开启)
    }
}

/**
 * @brief 定时器中断回调函数
 * @param htim
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(CAN_sched.htim != NULL && htim == CAN_sched.htim)  // 明确指定
    {
        CAN_Scheduler_TimerCallback();
    }
}