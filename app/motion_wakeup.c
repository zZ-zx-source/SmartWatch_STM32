#include "stm32f10x.h"                  // Device header
#include "motion_wakeup.h"
#include "MPU6050.h"       // MPU6050 ����
#include "auto_sleep.h"    // ���ڻ���
#include <math.h>          // ����Ƕ�
#include <stdio.h>         //NULL

static uint8_t motion_flag = 0;   // �� TIM2 �ж���λ

void Motion_Init(void)
{
    MPU6050_Init();        // ��ʼ��������
    motion_flag = 0;
}

/*�� TIM2 �жϵ��ã����ü���־*/
void Motion_SetFlag(void)
{
    motion_flag = 1;
}

/*��̧�ּ�⣺������ٶȼ�����������ļн�*/
/*
static uint8_t IsWristRaised(void)
{
    int16_t ax, ay, az;
    MPU6050_GetData(&ax, &ay, &az, NULL, NULL, NULL);

    // ����ϼ��ٶ�������Z��ļнǣ�����Z�ᴹֱ���ϣ�
    float norm = sqrtf(ax*ax + ay*ay + az*az);
    if (norm < 0.01f) return 0;

    float cos_theta = az / norm;   // Z�����ռ��
    // cos_theta < cos(45��) �� 0.707 ��ʾ��б����45��
    if (cos_theta < 0.707f) return 1;
    return 0;
}
*/
/*���ж������任һ�£�ֻ�������˷��ͱȽϡ�
ԭ������
cos��=az/((ax^2+ay^2+az^2)��ƽ����)<cos45=Լ����0.707
����ƽ����ע�� az ����Ϊ������ƽ�����������
az^2<0.5��(ax^2+ay^2+az^2)
�ȼ��ڣ�2��az^2<ax^2+ay^2+az^2
����az^2<ax^2+ay^2
*/
static uint8_t IsWristRaised(void)
{
    int16_t ax, ay, az;
    MPU6050_GetData(&ax, &ay, &az, NULL, NULL, NULL);

    // �� int32_t ��ֹ int16_t ������
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

/*����ѭ���е��ã�����Ƿ���Ҫ����*/
void Motion_Process(void)
{
    if (!motion_flag) return;
    motion_flag = 0;

    if (AutoSleep_IsSleeping() && IsWristRaised())
    {
        AutoSleep_Wakeup();     // ������Ļ
        // ���Ѻ���ѭ������� Menu_Update ˢ�½���
    }
}
