#include "Key.h"
#include "main.h"

/* 全局变量，存储按键键码 */
uint8_t Key_Num = 0;

void Key_Init(void)
{
    /* CubeMX 已把 K1~K4 配成上拉输入，这里留空 */
}

/* 读取键码并清零 */
uint8_t Key_GetNum(void)
{
    uint8_t Temp;
    if (Key_Num)
    {
        Temp = Key_Num;
        Key_Num = 0;
        return Temp;
    }
    return 0;
}

/* 获取当前按键状态（按住期间一直返回键码） */
uint8_t Key_GetState(void)
{
    if (HAL_GPIO_ReadPin(K_1_GPIO_Port, K_1_Pin) == GPIO_PIN_RESET) return 1;
    if (HAL_GPIO_ReadPin(K_2_GPIO_Port, K_2_Pin) == GPIO_PIN_RESET) return 2;
    if (HAL_GPIO_ReadPin(K_3_GPIO_Port, K_3_Pin) == GPIO_PIN_RESET) return 3;
    if (HAL_GPIO_ReadPin(K_4_GPIO_Port, K_4_Pin) == GPIO_PIN_RESET) return 4;
    return 0;
}

/* 按键扫描状态机（每 1ms 调用一次） */
void Key_Tick(void)
{
    static uint8_t Count = 0;
    static uint8_t CurrState = 0;
    static uint8_t PrevState = 0;

    Count++;
    if (Count >= 20)                 /* 20ms 进一次 */
    {
        Count = 0;
        PrevState = CurrState;
        CurrState = Key_GetState();

        /* 检测"松手瞬间"：本次无按键，上次有按键 */
        if (CurrState == 0 && PrevState != 0)
        {
            Key_Num = PrevState;
        }
    }
}