/*//////////////////////////////////////////////////////////////
define the Tasks' Stk lengh Prio Function on the project
by: tangxiaofeng xidian 503
//////////////////////////////////////////////////////////////*/

#ifndef APP_CFG_H
#define APP_CFG_H


//Task Stk lengh

#define	MainTaskStkLengh	1024*2   	// Define the MainTask stack length 
#define	Task0StkLengh		1024     	// Define the Task0 stack length 



//Task function
void    OSMainTask(void *pdata);   		//MainTask task
void 	Task0(void *pdata);			    // Task0 


//Task Prio
#define NormalTaskPrio       5
#define MainTaskPrio 	NormalTaskPrio
#define Task0Prio 		NormalTaskPrio+1

#endif