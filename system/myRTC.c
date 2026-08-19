#include "stm32f10x.h"                  // Device header
#include "myRTC.h"
#include <time.h>

uint16_t MyRTC_Time[] = {2026, 7, 31, 6, 5, 0};	//定义全局的时间数组，数组内容分别为年、月、日、时、分、秒
uint8_t MyRTC_Weekday = 0;   // 全局变量，存储星期几(0~6,对应周日~周六)

void MyRTC_Init(void)
{
	//开启时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);//开启PWR时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP,ENABLE);//开启BKP时钟

	//备份寄存器访问使能
	PWR_BackupAccessCmd(ENABLE);//使用PWR开启对备份寄存器的访问
	
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)        //通过写入备份寄存器的标志位，来判断RTC是否为第一次配置
	{                                                    //if成立则执行第一次RTC的配置
		RCC_LSEConfig(RCC_LSE_ON);//开启LSE时钟
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);//等待LSE准备就绪
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);          //选择RTCCLK来源为LSE
		RCC_RTCCLKCmd(ENABLE);                           //RTCCLK使能
		
		RTC_WaitForSynchro();                            //等待同步
		RTC_WaitForLastTask();                           //等待上一次操作完成
		
		RTC_SetPrescaler(32768 - 1);                     //设置RTC预分频器，预分频后的计数频率为1Hz
		RTC_WaitForLastTask();                           //等待上一次操作完成
		
		MyRTC_SetTime();
		
		BKP_WriteBackupRegister(BKP_DR1,0xA5A5);         //在备份寄存器中写入自己规定的标志位，来判断RTC是否为第一次配置
	}
	else
	{
		RTC_WaitForSynchro();                            //等待同步
		RTC_WaitForLastTask();                           //等待上一次操作完成
	}	
}

/*
//如果LSE无法起振导致程序卡死在初始化函数中
//可将初始化函数替换为下述代码，使用LSI当作RTCCLK
//LSI无法由备用电源供电，故主电源掉电时，RTC走时会暂停
 
void MyRTC_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);
	
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		RCC_LSICmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
		
		RTC_SetPrescaler(40000 - 1);
		RTC_WaitForLastTask();
		
		MyRTC_SetTime();
		
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		RCC_LSICmd(ENABLE);				//即使不是第一次配置，也需要再次开启LSI时钟
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
	}
}
*/

/*RTC设置时间*/
void MyRTC_SetTime(void)
{
	time_t time_cnt;		//定义秒计数器数据类型
	struct tm time_date;	//定义日期时间数据类型
	
	time_date.tm_year = MyRTC_Time[0] - 1900;		//将数组的时间赋值给日期时间结构体
	time_date.tm_mon = MyRTC_Time[1] - 1;
	time_date.tm_mday = MyRTC_Time[2];
	time_date.tm_hour = MyRTC_Time[3];
	time_date.tm_min = MyRTC_Time[4];
	time_date.tm_sec = MyRTC_Time[5];
	
	time_cnt = mktime(&time_date) - 8 * 60 * 60;	//调用mktime函数，将日期时间转换为秒计数器格式
													//- 8 * 60 * 60为东八区的时区调整
	
	RTC_SetCounter(time_cnt);						//将秒计数器写入到RTC的CNT中
	RTC_WaitForLastTask();	
}

/*RTC读时间*/
void MyRTC_ReadTime(void)
{
	time_t time_cnt;		//定义秒计数器数据类型
	struct tm time_date;	//定义日期时间数据类型
	
	time_cnt = RTC_GetCounter() + 8 * 60 * 60;		//读取RTC的CNT，获取当前的秒计数器
													//+ 8 * 60 * 60为东八区的时区调整
	
	time_date = *localtime(&time_cnt);				//使用localtime函数，将秒计数器转换为日期时间格式
	
	MyRTC_Time[0] = time_date.tm_year + 1900;		//将日期时间结构体赋值给数组的时间
	MyRTC_Time[1] = time_date.tm_mon + 1;
	MyRTC_Time[2] = time_date.tm_mday;
	MyRTC_Time[3] = time_date.tm_hour;
	MyRTC_Time[4] = time_date.tm_min;
	MyRTC_Time[5] = time_date.tm_sec;
	MyRTC_Weekday = time_date.tm_wday;   // 从 localtime 的结果中获取星期
}

