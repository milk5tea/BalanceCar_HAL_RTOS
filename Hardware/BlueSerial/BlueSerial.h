#ifndef __BLUESERIAL_H
#define __BLUESERIAL_H
#include "main.h"

#define BLUESERIAL_PACKET_MAX   100

void BlueSerial_Init(void);
void BlueSerial_SendByte(uint8_t Byte);
void BlueSerial_SendArray(uint8_t *Array, uint16_t Length);
void BlueSerial_SendString(char *String);
void BlueSerial_SendNumber(uint32_t Number, uint8_t Length);
void BlueSerial_Printf(char *format, ...);
void BlueSerial_Printf_DMA(const char *format, ...);

void BlueSerial_PutByte(uint8_t Byte);

/* ★ 新增：供 USART2_IRQHandler 调用 */
void BlueSerial_IdleHandler(void);

extern char    BlueSerial_RxPacket[BLUESERIAL_PACKET_MAX];
extern uint8_t BlueSerial_RxFlag;

#endif