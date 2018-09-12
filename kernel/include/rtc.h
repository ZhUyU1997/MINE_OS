#ifndef __RTC_H_
#define __RTC_H_
/*RTC初始化*/
void RTC_Init(void);
/*RTC设置*/
void RTC_Set(char year, char month, char date, char week, char hour, char min, char sec) ;
/*读取RTC*/
void RTC_Read(char *year, char *month, char *date, char *week, char *hour, char *min, char *sec);
/*RTC tick时钟设置*/
void RTC_Tick(void);
/*RTC 闹钟初始化*/
void RTC_Alarm(void);
/*RTC 闹钟设置*/
void RTC_Alarm_Set(char year, char mon, char date, char hour, char min, char sec) ;
#endif