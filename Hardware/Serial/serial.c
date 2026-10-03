/***************************************************************************************
  * 文件名称：Serial.c
  * 功能描述：USART1 调试串口驱动（HAL 库版本）
  * 引脚分配：PA9 = TX, PA10 = RX
  * 波特率：115200, 8N1
  ***************************************************************************************/

#include "Serial.h"
#include <stdio.h>
#include <stdarg.h>


extern UART_HandleTypeDef huart1;

uint8_t Serial_RxData = 0;
uint8_t Serial_RxFlag = 0;

static uint8_t Serial_RxBuffer = 0;

void Serial_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &Serial_RxBuffer, 1);
}

void Serial_SendByte(uint8_t Byte)
{
    HAL_UART_Transmit(&huart1, &Byte, 1, HAL_MAX_DELAY);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    HAL_UART_Transmit(&huart1, Array, Length, HAL_MAX_DELAY);
}

void Serial_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        Serial_SendByte((uint8_t)String[i]);
    }
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
    }
}

int fputc(int ch, FILE *f)
{
    Serial_SendByte((uint8_t)ch);
    return ch;
}

void Serial_Printf(char *format, ...)
{
    char String[128];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial_SendString(String);
}

uint8_t Serial_GetRxFlag(void)
{
    if (Serial_RxFlag == 1)
    {
        Serial_RxFlag = 0;
        return 1;
    }
    return 0;
}

uint8_t Serial_GetRxData(void)
{
    return Serial_RxData;
}

/* ============================================================
 * ★ 修改：只保留 USART1 分支，删掉 USART2 分支
 * （USART2 现在由 BlueSerial.c 的 BlueSerial_IdleHandler 处理）
 * ============================================================ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Serial_RxData = Serial_RxBuffer;
        Serial_RxFlag = 1;
        HAL_UART_Receive_IT(&huart1, &Serial_RxBuffer, 1);
    }
    /* USART2 分支已删掉 ★ */
}