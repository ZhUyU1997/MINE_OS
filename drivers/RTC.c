#include <interrupt.h>
#include <s3c24xx.h>
/*定义用户中断标志*/
void RTC_alarm_interrupt(void);
void RTC_tick_interrupt(void);

/*RTC初始化*/
void RTC_Init(void) {
	request_irq(INT_TICK, RTC_tick_interrupt); //RTC 时钟滴答中断
	request_irq(INT_RTC, RTC_alarm_interrupt); //RTC 闹钟中断
}

/*RTC设置*/
void RTC_Set(char year, char month, char date,
			 char week, char hour, char min, char sec) {
	rRTCCON		= 1;

	rBCDYEAR	= year;
	rBCDMON		= ((month / 10) << 4) | (month % 10);
	rBCDDAY		= week;
	rBCDDATE	= ((date  / 10) << 4) | (date  % 10);
	rBCDHOUR	= ((hour  / 10) << 4) | (hour  % 10);
	rBCDMIN		= ((min   / 10) << 4) | (min   % 10);
	rBCDSEC		= ((sec   / 10) << 4) | (sec   % 10);

	rRTCCON		&= ~1;
}

/*读取RTC*/
void RTC_Read(char *year, char *month, char *date,
			  char *week, char *hour, char *min, char *sec) {
	char		i;

	rRTCCON		= 1;

	*year		= rBCDYEAR;
	*month		= (rBCDMON  >> 4) * 10 + (rBCDMON  & 0x0f);
	*week		= rBCDDAY;
	*date		= (rBCDDATE >> 4) * 10 + (rBCDDATE & 0x0f);
	*hour		= (rBCDHOUR >> 4) * 10 + (rBCDHOUR & 0x0f);
	*min		= (rBCDMIN  >> 4) * 10 + (rBCDMIN  & 0x0f);

	if ((*sec = rBCDSEC) == 0) {
		*year	= rBCDYEAR;
		*month	= (rBCDMON  >> 4) * 10 + (rBCDMON  & 0x0f);
		*week	= rBCDDAY;
		*date	= (rBCDDATE >> 4) * 10 + (rBCDDATE & 0x0f);
		*hour	= (rBCDHOUR >> 4) * 10 + (rBCDHOUR & 0x0f);
		*min	= (rBCDMIN  >> 4) * 10 + (rBCDMIN  & 0x0f);
	}

	*sec		= (rBCDSEC  >> 4) * 10 + (rBCDSEC  & 0x0f);

	rRTCCON &= ~1;
}

/*RTC tick时钟设置*/
void RTC_Tick(void) {
	rTICNT 		&= ~0xff;
	rTICNT 		|= (1 << 7 | 63);
}

/*RTC 闹钟初始化*/
void RTC_Alarm(void) {
	rRTCALM 	&= ~0xff;
	rRTCALM 	|= 0xff;
}

/*RTC 闹钟设置*/
void RTC_Alarm_Set(char year, char mon, char date,
				   char hour, char min, char sec) {
	rALMYEAR	= year;
	rALMMON		= ((mon  / 10) << 4) | mon  % 10;
	rALMDATE	= ((date / 10) << 4) | date % 10;
	rALMHOUR	= ((hour / 10) << 4) | hour % 10;
	rALMMIN		= ((min  / 10) << 4) | min  % 10;
	rALMSEC		= ((sec  / 10) << 4) | sec  % 10;
}

/*RTC 闹钟中断服务*/
void RTC_alarm_interrupt(void) {

}

/*RTC tick中断服务*/
void RTC_tick_interrupt(void) {

}

