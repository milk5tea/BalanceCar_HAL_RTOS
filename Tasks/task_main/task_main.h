#ifndef __TASK_MAIN_H
#define __TASK_MAIN_H

#include "main.h"

/* 任务初始化（在主循环前调用一次） */
void Task_Init(void);

/* 主任务（在主循环里反复调用） */
void Task_Main(void);

/* 1ms 心跳中断回调（放在 HAL_TIM_PeriodElapsedCallback 里调用） */
void Task_1ms_Tick(void);

#endif