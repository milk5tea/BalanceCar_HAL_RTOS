/*pwm.c*/
#include "pwm.h"
#include "tim.h"

void PWM_Init()
{
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
}

void PWM_SetCompare1(uint16_t Compare)
{
    __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, Compare);
}

void PWM_SetCompare2(uint16_t Compare)
{
    __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, Compare);
}

