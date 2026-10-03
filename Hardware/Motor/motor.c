/*motor.c*/
#include "motor.h"
#include "pwm.h"

void Motor_Init(void)
{
    PWM_Init();
}

void Motor_SetPWM(uint8_t motor_id, int16_t pwm)
{
    /* 限幅，防止越界导致驱动异常 */
    if (pwm >  100) pwm =  100;
    if (pwm < -100) pwm = -100;

    if(motor_id==1)              //左轮
    {
        if(pwm>=0)
        {
            HAL_GPIO_WritePin( AIN_1_GPIO_Port,  AIN_1_Pin,  GPIO_PIN_SET);
            HAL_GPIO_WritePin(AIN_2_GPIO_Port, AIN_2_Pin ,  GPIO_PIN_RESET);
            PWM_SetCompare1(pwm);
        }
        else{
            HAL_GPIO_WritePin(AIN_1_GPIO_Port,AIN_1_Pin,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AIN_2_GPIO_Port,AIN_2_Pin, GPIO_PIN_SET);
            PWM_SetCompare1(-pwm);
        }
    }
    else if(motor_id==2)         //右轮
    {
        if(pwm>=0)
        {
            HAL_GPIO_WritePin( BIN_1_GPIO_Port,  BIN_1_Pin,  GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BIN_2_GPIO_Port, BIN_2_Pin ,  GPIO_PIN_SET);
            PWM_SetCompare2(pwm);
        }
        else{
            HAL_GPIO_WritePin(BIN_1_GPIO_Port,BIN_1_Pin,GPIO_PIN_SET);
            HAL_GPIO_WritePin(BIN_2_GPIO_Port,BIN_2_Pin, GPIO_PIN_RESET);
            PWM_SetCompare2(-pwm);
        }
    }
}