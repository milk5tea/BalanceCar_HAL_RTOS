/* pwm.h */
#ifndef __PWM_H       
#define __PWM_H       
#include "main.h"

void PWM_Init(void);
void PWM_SetCompare1(uint16_t Compare); // 对应 PA0 (左轮)
void PWM_SetCompare2(uint16_t Compare); // 对应 PA1 (右轮)

#endif                // 结束判断
