#include "stm32f10x.h"                  // Device header
#include "motion_wakeup.h"
#include "MPU6050.h"       // MPU6050 驱动
#include "auto_sleep.h"    // 用于唤醒
#include <math.h>          // 计算角度
#include <stdio.h>         //NULL

static uint8_t motion_flag = 0;   // 由 TIM2 中断置位

void Motion_Init(void)
{
    MPU6050_Init();        // 初始化传感器
    motion_flag = 0;
}

/*由 TIM2 中断调用，设置检测标志*/
void Motion_SetFlag(void)
{
    motion_flag = 1;
}

/*简单抬手检测：计算加速度计与重力方向的夹角*/
/*
static uint8_t IsWristRaised(void)
{
    int16_t ax, ay, az;
    MPU6050_GetData(&ax, &ay, &az, NULL, NULL, NULL);

    // 计算合加速度向量与Z轴的夹角（假设Z轴垂直向上）
    float norm = sqrtf(ax*ax + ay*ay + az*az);
    if (norm < 0.01f) return 0;

    float cos_theta = az / norm;   // Z轴分量占比
    // cos_theta < cos(45°) ≈ 0.707 表示倾斜超过45度
    if (cos_theta < 0.707f) return 1;
    return 0;
}
*/
/*把判断条件变换一下，只用整数乘法和比较。
原条件：
cosθ=az/((ax^2+ay^2+az^2)的平方根)<cos45=约等于0.707
两边平方（注意 az 可能为负，但平方后成立）：
az^2<0.5×(ax^2+ay^2+az^2)
等价于：2×az^2<ax^2+ay^2+az^2
即：az^2<ax^2+ay^2
*/
static uint8_t IsWristRaised(void)
{
    int16_t ax, ay, az;
    MPU6050_GetData(&ax, &ay, &az, NULL, NULL, NULL);

    // 用 int32_t 防止 int16_t 相乘溢出
    int32_t ax2 = (int32_t)ax * ax;
    int32_t ay2 = (int32_t)ay * ay;
    int32_t az2 = (int32_t)az * az;

    // 判断 Z 轴分量是否小于水平分量，等价于倾斜角 >45°
    if (az2 < ax2 + ay2) 
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*在主循环中调用，检查是否需要唤醒*/
void Motion_Process(void)
{
    if (!motion_flag) return;
    motion_flag = 0;

    if (AutoSleep_IsSleeping() && IsWristRaised())
    {
        AutoSleep_Wakeup();     // 唤醒屏幕
        // 唤醒后主循环会调用 Menu_Update 刷新界面
    }
}
