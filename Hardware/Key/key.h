#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/* 按键编号 */
#define KEY_1   1
#define KEY_2   2
#define KEY_3   3
#define KEY_4   4

void Key_Init(void);
uint8_t Key_GetNum(void);
uint8_t Key_GetState(void);
void Key_Tick(void);

#endif