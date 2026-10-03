#include "main.h"
#include "filter.h"
#include <math.h>

/* 定义 π 常量（防止某些编译器不识别 M_PI） */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/**
  * 函    数：互补滤波器初始化
  * 参    数：f      滤波器结构体指针
  * 参    数：alpha  互补滤波系数，0~1，典型值 0.98
  * 参    数：dt     采样周期（秒），例如 5ms 写 0.005f
  */
void Filter_Init(ComplementaryFilter_t *f, float alpha, float dt)
{
    f->Angle = 0.0f;
    f->Alpha = alpha;
    f->Dt    = dt;
}

/**
  * 函    数：设置滤波器初始角度
  * 说    明：开机时用加速度计算一次角度，给陀螺仪积分一个正确的起点
  */
void Filter_SetAngle(ComplementaryFilter_t *f, float init_angle)
{
    f->Angle = init_angle;
}

/**
  * 函    数：互补滤波更新（每 dt 秒调用一次）
  * 
  * 数学原理：
  *   Angle(k) = α * [Angle(k-1) + Gyro * dt] + (1 - α) * Acc_Angle
  * 
  *   - 陀螺仪积分项：动态响应快，但长期漂移
  *   - 加速度计角度：静态准确，但噪声大
  *   - α 越大 → 越信任陀螺仪（响应快，但漂移抑制弱）
  *   - α 越小 → 越信任加速度计（抗漂移强，但噪声大）
  * 
  *   典型 α = 0.98，dt = 0.005s（5ms）
  */
float Filter_Update(ComplementaryFilter_t *f, float gyro_rate, float acc_angle)
{
    f->Angle = f->Alpha * (f->Angle + gyro_rate * f->Dt) 
             + (1.0f - f->Alpha) * acc_angle;
    return f->Angle;
}

/**
  * 函    数：从加速度计的 X、Z 轴原始值计算俯仰角
  * 参    数：acc_x  加速度计 X 轴原始值
  * 参    数：acc_z  加速度计 Z 轴原始值
  * 返 回 值：角度（度），范围 -180° ~ +180°
  * 说    明：模块平放时 AZ 最大、AX≈0，角度≈0°；模块前倾时 AX 变化，角度随之变化
  */
float Filter_AccToAngle(float acc_x, float acc_z)
{
    return atan2f(acc_x, acc_z) * 180.0f / M_PI;
}