#ifndef __FILTER_H
#define __FILTER_H

#include "main.h"

/**
  * 互补滤波器结构体
  * 保存滤波器的所有状态变量，方便同时使用多个滤波器
  */
typedef struct {
    float Angle;    // 融合后的角度（度）
    float Alpha;    // 互补滤波系数（0~1，越大越信任陀螺仪）
    float Dt;       // 采样周期（秒）
} ComplementaryFilter_t;

/* 初始化滤波器：指定系数 α 和采样周期 dt */
void Filter_Init(ComplementaryFilter_t *f, float alpha, float dt);

/* 设置初始角度（用加速度计计算一次，给滤波器一个起点） */
void Filter_SetAngle(ComplementaryFilter_t *f, float init_angle);

/**
  * 互补滤波更新
  * @param  gyro_rate  陀螺仪角速度（度/秒）
  * @param  acc_angle  加速度计计算出的角度（度）
  * @return 融合后的角度（度）
  */
float Filter_Update(ComplementaryFilter_t *f, float gyro_rate, float acc_angle);

/**
  * 从加速度计的 X、Z 轴原始值计算角度（度）
  * 公式：θ = atan2(acc_x, acc_z) * 180 / π
  */
float Filter_AccToAngle(float acc_x, float acc_z);

#endif