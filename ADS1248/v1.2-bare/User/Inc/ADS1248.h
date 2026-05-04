//
// Created by emmqa on 2026/1/30.
//

#ifndef SENSOR_MODULE_ADS1248_H
#define SENSOR_MODULE_ADS1248_H

#include "../../Core/Inc/main.h"
#include "../../Core/Inc/spi.h"

#define ADS1248_DRDY_PIN GPIO_PIN_0
#define ADS1248_DRDY_PORT GPIOB
#define ADS1248_CS_PIN GPIO_PIN_1
#define ADS1248_CS_PORT GPIOB

typedef enum
{
    ADS1248_SPI_IDLE = 0,
    ADS1248_SPI_BUSY
} ADS1248_SPI_State_t;

#define ADS1248_MUX0 0x00       //多路复用器控制寄存器0
#define ADS1248_VBIAS 0x01      //偏置电压寄存器
#define ADS1248_MUX1 0x02       //多路复用器控制寄存器1
#define ADS1248_SYS0 0x03       //系统控制寄存器
#define ADS1248_OFC0 0x04       //偏移校准系数寄存器0
#define ADS1248_OFC1 0x05       //偏移校准系数寄存器1
#define ADS1248_OFC2 0x06       //偏移校准系数寄存器2
#define ADS1248_FSC0 0x07       //全量程校准系数寄存器0
#define ADS1248_FSC1 0x08       //全量程校准系数寄存器1
#define ADS1248_FSC2 0x09       //全量程校准系数寄存器2
#define ADS1248_IDAC0 0x0A      //控制寄存器0
#define ADS1248_IDAC1 0x0B      //控制寄存器1
#define ADS1248_GPIOCFG 0x0C    //GPIO配置寄存器
#define ADS1248_GPIODIR 0x0D    //GPIO方向寄存器
#define ADS1248_GPIODAT 0x0E    //GPIO数据寄存器

/* SPI Commands */
#define ADS1248_WAKEUP 0x00         //唤醒
#define ADS1248_SLEEP 0x02          //休眠
#define ADS1248_SYNC 0x04           //重置ADC数字滤波器并启动新一轮转换
#define ADS1248_RESET 0x06          //复位
#define ADS1248_RDATA 0x12          //将最新的转换结果载入输出寄存器
#define ADS1248_RDATAC 0x14         //开启读数据连续模式
#define ADS1248_SDATAC 0x16         //终止读数据连续模式

/* 读写寄存器基础操作码 */
#define ADS1248_RREG 0x20           //格式: (0x20 | addr << 8) | (BytesNum - 1)
#define ADS1248_WREG 0x40           //格式: (0x40 | addr << 8) | (BytesNum - 1)

/* 系统校准命令 */
#define ADS1248_SYSOCAL 0x60        //启动系统偏移校准
#define ADS1248_SYSGCAL 0x61        //启动系统增益校准
#define ADS1248_SELFOCAL 0x62       //启动自偏移校准

#define ADS1248_NOP 0xFF            //这是一个无操作指令，用于在不输入指令的情况下输出数据
#define ADS1248_Restricted_Command 0xF1     //受限命令，不能发送给设备

/* MUX1 */

/* MUX1 寄存器位域掩码 */
#define ADS1248_MUX1_VREFCON_MASK   0x60  // Bit 6:5
#define ADS1248_MUX1_REFSELT_MASK   0x18  // Bit 4:3
#define ADS1248_MUX1_MUXCAL_MASK    0x07  // Bit 2:0

/* VREFCON: 内部基准控制 6:5 */
#define ADS1248_VREF_OFF      (0x00 << 5)  // 关闭内部基准
#define ADS1248_VREF_ON       (0x01 << 5)  // 始终开启内部基准 (推荐用于 10V 测量)
#define ADS1248_VREF_CONV     (0x02 << 5)  // 仅在转换时开启 (省电模式)

/* REFSELT: 参考源选择 4:3 */
#define ADS1248_REF_EXT0      (0x00 << 3)  // 使用外部基准对 0
#define ADS1248_REF_EXT1      (0x01 << 3)  // 使用外部基准对 1
#define ADS1248_REF_INT       (0x02 << 3)  // 使用内部基准 (不输出到引脚)
#define ADS1248_REF_INT_OUT   (0x03 << 3)  // 11: 使用内部参考并输出到 REFP0/N0

/* MUXCAL: 诊断模式 2:0*/
#define ADS1248_MON_NORMAL      (0x00)       // 正常操作模式
#define ADS1248_MON_OFFSET_CAL  (0x01)       // 偏移校准：内部短路至(AVDD+AVSS)/2
#define ADS1248_MON_GAIN_CAL    (0x02)       // 增益校准：连接至电压基准
#define ADS1248_MON_TEMP        (0x03)       // 温度测量：连接内部温度二极管
#define ADS1248_MON_REF1        (0x04)       // 监控外部参考 REF1
#define ADS1248_MON_REF0        (0x05)       // 监控外部参考 REF0
#define ADS1248_MON_AVDD        (0x06)       // 监控模拟电源 (AVDD-AVSS)/4
#define ADS1248_MON_DVDD        (0x07)       // 监控数字电源 (DVDD-DGND)/4

/* SYS0 */

/* SYS0 寄存器位域掩码 */
#define ADS1248_SYS0_PGA_MASK    0x70  // Bit 6:4
#define ADS1248_SYS0_DR_MASK     0x0F  // Bit 3:0

/* PGA 增益设置 (Bit 6:4) */
#define ADS1248_PGA_1            (0x00 << 4)
#define ADS1248_PGA_2            (0x01 << 4)
#define ADS1248_PGA_4            (0x02 << 4)
#define ADS1248_PGA_8            (0x03 << 4)
#define ADS1248_PGA_16           (0x04 << 4)
#define ADS1248_PGA_32           (0x05 << 4)
#define ADS1248_PGA_64           (0x06 << 4)
#define ADS1248_PGA_128          (0x07 << 4)

/* DR 数据速率设置 (Bit 3:0) */
#define ADS1248_DR_5SPS          (0x00)
#define ADS1248_DR_10SPS         (0x01)
#define ADS1248_DR_20SPS         (0x02)
#define ADS1248_DR_40SPS         (0x03)
#define ADS1248_DR_80SPS         (0x04)
#define ADS1248_DR_160SPS        (0x05)
#define ADS1248_DR_320SPS        (0x06)
#define ADS1248_DR_640SPS        (0x07)
#define ADS1248_DR_1000SPS       (0x08)
#define ADS1248_DR_2000SPS       (0x09) // 1001 to 1111 均为 2000SPS

/* IDAC0 */

/* IDAC0 寄存器位掩码 */
#define ADS1248_IDAC0_DRDY_MODE_MASK (0x08)
#define ADS1248_IDAC0_IMAG_MASK      (0x07)

/* DRDY_MODE设置 (Bit 3) */
#define ADS1248_IDAC0_DRDY_OFF       (0x00 << 3)       //仅输出
#define ADS1248_IDAC0_DRDY_ON        (0x01 << 3)       //兼容DRDY功能

/* IMAG IDAC 电流大小 (Bit 2:0) */
#define ADS1248_IDAC0_IMAG_OFF     (0x00)     //关闭
#define ADS1248_IDAC0_IMAG_50uA    (0x01)     //50uA
#define ADS1248_IDAC0_IMAG_100uA   (0x02)     //100uA
#define ADS1248_IDAC0_IMAG_250uA   (0x03)     //250uA
#define ADS1248_IDAC0_IMAG_500uA   (0x04)     //500uA
#define ADS1248_IDAC0_IMAG_750uA   (0x05)     //750uA
#define ADS1248_IDAC0_IMAG_1000uA  (0x06)     //1000uA
#define ADS1248_IDAC0_IMAG_1500uA  (0x07)     //1500uA

/* IDAC1 */

/* IDAC1 寄存器位掩码 */
#define ADS1248_IDAC1_I1DIR_MASK   (0xF0)
#define ADS1248_IDAC1_I2DIR_MASK   (0x0F)

/* I1DIR 激励电流源输出引脚选择 */
#define ADS1248_IDAC1_I1DIR_AIN0    (0x00 << 4)
#define ADS1248_IDAC1_I1DIR_AIN1    (0x01 << 4)
#define ADS1248_IDAC1_I1DIR_AIN2    (0x02 << 4)
#define ADS1248_IDAC1_I1DIR_AIN3    (0x03 << 4)
#define ADS1248_IDAC1_I1DIR_AIN4    (0x04 << 4)
#define ADS1248_IDAC1_I1DIR_AIN5    (0x05 << 4)
#define ADS1248_IDAC1_I1DIR_AIN6    (0x06 << 4)
#define ADS1248_IDAC1_I1DIR_AIN7    (0x07 << 4)
#define ADS1248_IDAC1_I1DIR_IEXC1    (0x08 << 4)
#define ADS1248_IDAC1_I1DIR_IEXC2    (0x09 << 4)
#define ADS1248_IDAC1_I1DIR_Disconnected    (0x0F << 4)  //断开连接

/* I2DIR 激励电流源输出引脚选择 */
#define ADS1248_IDAC1_I2DIR_AIN0    (0x00)
#define ADS1248_IDAC1_I2DIR_AIN1    (0x01)
#define ADS1248_IDAC1_I2DIR_AIN2    (0x02)
#define ADS1248_IDAC1_I2DIR_AIN3    (0x03)
#define ADS1248_IDAC1_I2DIR_AIN4    (0x04)
#define ADS1248_IDAC1_I2DIR_AIN5    (0x05)
#define ADS1248_IDAC1_I2DIR_AIN6    (0x06)
#define ADS1248_IDAC1_I2DIR_AIN7    (0x07)
#define ADS1248_IDAC1_I2DIR_IEXC1    (0x08)
#define ADS1248_IDAC1_I2DIR_IEXC2    (0x09)
#define ADS1248_IDAC1_I2DIR_Disconnected    (0x0F)      //断开连接

/**
 * @brief 配置多路复用器控制寄存器0
 * @param GPIOx: 对应芯片的 CS 所在端口
 * @param GPIO_Pin: 对应芯片的 CS 引脚号
 * @param Mode: 4种差分对组合
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_SetMUX0(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t Mode);

/**
 * @brief  读取并换算 10V 分压后的实际电压
 * @param  GPIOx: CS 端口
 * @param  GPIO_Pin: CS 引脚
 * @param  ratio: 外部分压比例 (例如测量10V分压到2V，ratio则为 5.0)
 * @return float: 返回实际测得的电压值 (单位: V)
 */
float ADS1248_Measure_Voltage(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, float ratio);

/**
 * @brief ADS1248初始化
 * @param GPIOx: CS 端口
 * @param GPIO_Pin: CS 引脚
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADS1248_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif //SENSOR_MODULE_ADS1248_H