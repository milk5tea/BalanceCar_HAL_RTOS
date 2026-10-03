/*encoder.c*/
#include "encoder.h"
#include "tim.h"

void Encoder_Init()
{
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL );
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL );

    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
}

int16_t Encoder_Get(uint8_t n)
{
    int16_t temp;

    if(n==1)
    {
        temp=(int16_t)__HAL_TIM_GET_COUNTER(&htim3);
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        return temp;
    }
    else if (n==2) {
        temp=(int16_t)__HAL_TIM_GET_COUNTER(&htim4);
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        return temp;
    }
    return 0;  // 参数n非法，默认返回0
}

