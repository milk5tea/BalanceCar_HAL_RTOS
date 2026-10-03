#include "main.h"                  // Device header

/*引脚配置层*/

/**
  * 函    数：I2C写SCL引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SCL的电平，范围0~1
  * 返 回 值：无
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (GPIO_PinState)BitValue);
    for (volatile int i = 0; i < 20; i++); // 加一点点延时降速
}

/**
  * 函    数：I2C写SDA引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SDA的电平，范围0~1
  * 返 回 值：无
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, (GPIO_PinState)BitValue);
    for (volatile int i = 0; i < 20; i++); // 加一点点延时降速
}

/**
  * 函    数：I2C读SDA引脚电平
  * 参    数：无
  * 返 回 值：协议层需要得到的当前SDA的电平，范围0~1
  */
uint8_t MyI2C_R_SDA(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
}

/**
  * 函    数：I2C初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数仅供上层调用，底层GPIO初始化已在CubeMX生成的 MX_GPIO_Init() 中完成
  */
void MyI2C_Init(void)
{
    // 释放总线（拉高SCL和SDA）
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(1);
}

/*协议层*/

/**
  * 函    数：I2C起始
  */
void MyI2C_Start(void)
{
    MyI2C_W_SDA(1);							//释放SDA，确保SDA为高电平
    MyI2C_W_SCL(1);							//释放SCL，确保SCL为高电平
    MyI2C_W_SDA(0);							//在SCL高电平期间，拉低SDA，产生起始信号
    MyI2C_W_SCL(0);							//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  */
void MyI2C_Stop(void)
{
    MyI2C_W_SDA(0);							//拉低SDA，确保SDA为低电平
    MyI2C_W_SCL(1);							//释放SCL，使SCL呈现高电平
    MyI2C_W_SDA(1);							//在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * 函    数：I2C发送一个字节
  */
void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i ++)
    {
        MyI2C_W_SDA(!!(Byte & (0x80 >> i)));
        MyI2C_W_SCL(1);
        MyI2C_W_SCL(0);
    }
}

/**
  * 函    数：I2C接收一个字节
  */
uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    MyI2C_W_SDA(1);
    for (i = 0; i < 8; i ++)
    {
        MyI2C_W_SCL(1);
        if (MyI2C_R_SDA()) { Byte |= (0x80 >> i); }
        MyI2C_W_SCL(0);
    }
    return Byte;
}

/**
  * 函    数：I2C发送应答位
  */
void MyI2C_SendAck(uint8_t AckBit)
{
    MyI2C_W_SDA(AckBit);
    MyI2C_W_SCL(1);
    MyI2C_W_SCL(0);
}

/**
  * 函    数：I2C接收应答位
  */
uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t AckBit;
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    AckBit = MyI2C_R_SDA();
    MyI2C_W_SCL(0);
    return AckBit;
}