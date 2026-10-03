#include "BlueSerial.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef  hdma_usart2_rx;

/* ============= 对外暴露 ============= */
char    BlueSerial_RxPacket[BLUESERIAL_PACKET_MAX];
uint8_t BlueSerial_RxFlag = 0;

/* ============= DMA 接收缓冲 ============= */
#define BLUE_RX_BUF_SIZE 64
static uint8_t blue_rx_buf[BLUE_RX_BUF_SIZE];

/* ============================================================
 * BlueSerial_Init：DMA 循环接收 + IDLE 中断
 * ============================================================ */
void BlueSerial_Init(void)
{
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart2, blue_rx_buf, BLUE_RX_BUF_SIZE);
}

/* ============================================================
 * BlueSerial_IdleHandler：一帧数据到达
 *   DMA Circular 模式，不需要重启，只读新数据
 * ============================================================ */
void BlueSerial_IdleHandler(void)
{
    static uint16_t last_pos = 0;

    uint16_t remaining = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
    uint16_t curr_pos  = BLUE_RX_BUF_SIZE - remaining;

    /* 从 last_pos 到 curr_pos 的字节逐个送状态机 */
    while (last_pos != curr_pos)
    {
        BlueSerial_PutByte(blue_rx_buf[last_pos]);
        last_pos++;
        if (last_pos >= BLUE_RX_BUF_SIZE) last_pos = 0;
    }
    /* Circular 模式，不用手动重启 DMA */
}

/* ============================================================
 * 发送相关
 * ============================================================ */
void BlueSerial_SendByte(uint8_t Byte)
{
    HAL_UART_Transmit(&huart2, &Byte, 1, HAL_MAX_DELAY);
}

void BlueSerial_SendArray(uint8_t *Array, uint16_t Length)
{
    HAL_UART_Transmit(&huart2, Array, Length, HAL_MAX_DELAY);
}

void BlueSerial_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
        BlueSerial_SendByte((uint8_t)String[i]);
}

uint32_t BlueSerial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) Result *= X;
    return Result;
}

void BlueSerial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
        BlueSerial_SendByte(Number / BlueSerial_Pow(10, Length - i - 1) % 10 + '0');
}

void BlueSerial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    BlueSerial_SendString(String);
}

/* ============================================================
 * 状态机：逐字节解析 [MSG]
 * ============================================================ */
void BlueSerial_PutByte(uint8_t Byte)
{
    static uint8_t RxState = 0;
    static uint8_t pRxPacket = 0;

    if (RxState == 0)
    {
        if (Byte == '[' && BlueSerial_RxFlag == 0)
        {
            RxState = 1;
            pRxPacket = 0;
        }
    }
    else if (RxState == 1)
    {
        if (Byte == ']')
        {
            RxState = 0;
            BlueSerial_RxPacket[pRxPacket] = '\0';
            BlueSerial_RxFlag = 1;
        }
        else
        {
            if (pRxPacket < BLUESERIAL_PACKET_MAX - 1)
                BlueSerial_RxPacket[pRxPacket++] = Byte;
        }
    }
}

/* ============================================================
 * ORE 错误回调：Circular 模式不用重启 DMA
 * ============================================================ */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
    }
}

/* ============================================================
 * DMA 非阻塞发送（波形打印）
 * ============================================================ */
static uint8_t dma_tx_buf[64];
static volatile uint8_t dma_tx_busy = 0;

void BlueSerial_Printf_DMA(const char *format, ...)
{
    if (dma_tx_busy) return;

    char String[64];
    va_list arg;
    va_start(arg, format);
    int len = vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);

    if (len <= 0 || len > (int)sizeof(dma_tx_buf)) return;

    memcpy(dma_tx_buf, String, len);
    dma_tx_busy = 1;
    HAL_UART_Transmit_DMA(&huart2, dma_tx_buf, len);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
        dma_tx_busy = 0;
}