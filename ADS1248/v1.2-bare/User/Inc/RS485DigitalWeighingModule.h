//
// Created by emmqa on 2026/2/24.
//

#ifndef SENSOR_MODULE_BARE_RS485DIGITALWEIGHINGMODULE_H
#define SENSOR_MODULE_BARE_RS485DIGITALWEIGHINGMODULE_H

#include "modbus.h"

#define DWM_Real_Time_Weight 0x0000    //实时重量
#define DWM_AD_Internal_Code 0x0002    //内码值
#define DWM_Zero_Offset_Code 0x0004    //当前零位
#define DWM_Calibration_Weight_Value 0x0006    //砝码值
#define DWM_Zero_Tracking_Intensity 0x0009     //追零强度
#define DWM_Dynamic_Zero_Tracking_Range 0x000A //动态追零
#define DWM_Zero_Tracking_Enable 0X000B        //追零使能
#define DWM_Division_Value 0x000C      //分度值
#define DWM_Median_Filter 0X000D       //中值滤波
#define DWM_Sampling_Rate 0x000E       //采样速率
#define DWM_Slave_Address_Reg 0X000F   //模块地址
#define DWM_Baud_Rate 0X0010           //波特率
#define DWM_Average_Filter 0X0011      //平均滤波
#define DWM_Dynamic_Tracking_Range 0X0012      //动态跟踪范围
#define DWM_Creep_Tracking_Range 0x0013        //蠕变跟踪范围
#define DWM_Stable_Weight_Output_Switch 0X0014 //稳定重量开关
#define DWM_Tare 0x0015                 //去皮
#define DWM_Command_Register 0X0016     //命令寄存器
#define DWM_Write_Protection 0X0017     //写保护寄存器
#define DWM_Calibration_Mode_Select 0X0018      //校准方式选择
#define DWM_Multi_point_Calib_Weight1 0X0019    //第一点砝码值
#define DWM_Multi_point_Calib_Weight2 0X001A    //第二点砝码值
#define DWM_Multi_point_Calib_Weight3 0X001B    //第三点砝码值
#define DWM_Multi_point_Calib_Weight4 0X001C    //第四点砝码值
#define DWM_Multi_point_Calib_Weight5 0X001D    //第五点砝码值
#define DWM_Multi_point_Calib_Weight6 0X001E    //第六点砝码值
#define DWM_Multi_point_Calib_Weight7 0X001F    //第七点砝码值
#define DWM_Multi_point_Calib_Weight8 0X0020    //第八点砝码值
#define DWM_Multi_point_Calib_Weight9 0X0021    //第九点砝码值
#define DWM_Multi_point_Calib_Weight10 0X0022   //第十点砝码值
#define DWM_Sensor_Full_Capacity 0X0023         //传感器总量程
#define DWM_Sensor_Sensitivity 0X0024           //传感器灵敏度
#define DWM_Voltage_Correction_Ratio 0X0025     //电压修正比
#define DWM_Span_Correction_Coefficient 0x0026  //量程修正系数

#define DWM_Slave_Address 0x02

typedef enum
{
    DWM_OK = 0,
    DWM_ERROR,
    DWM_CRCERROR
} DWM_StatusTypedef;

extern int32_t DWM_RealWeight;

#endif //SENSOR_MODULE_BARE_RS485DIGITALWEIGHINGMODULE_H