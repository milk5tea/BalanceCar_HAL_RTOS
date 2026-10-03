/* motor.h */
#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"  // 必须包含，里面包含了HAL库定义和CubeMX生成的引脚宏

/* 函数声明 */
void Motor_Init(void);
void Motor_SetPWM(uint8_t motor_id, int16_t pwm);

#endif
