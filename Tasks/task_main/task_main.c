/* ============================================================
 * task_main.c
 * 平衡车主任务（角度环 + 速度环 + 转向环 三环串级 PID）
 * ============================================================ */

#include "task_main.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encoder.h"
#include "Key.h"
#include "Serial.h"
#include "BlueSerial.h"
#include "pid.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ========== 功能开关（先注释，后续启用改 1） ========== */
#define ENABLE_POS_LOOP      0   /* 位置环（四环串级） */
#define ENABLE_FEEDFORWARD   0   /* 前馈补偿 */

/* ========== 角度环 PID ========== */
PID_t AnglePID = {
    .Kp = 3.00,     .Ki = 0.08,     .Kd = 3.00,
    .OutMax = 100,  .OutMin = -100,
    .OutOffset = 3,
    .ErrorIntMax = 600, .ErrorIntMin = -600,
};

/* ========== 速度环 PID ========== */
PID_t SpeedPID = {
    .Kp = 1.35,     .Ki = 0.04,     .Kd = 0.02,
    .OutMax = 20,   .OutMin = -20,
    .ErrorIntMax = 150, .ErrorIntMin = -150,
};

/* ========== 转向环 PID ========== */
PID_t TurnPID = {
    .Kp = 4.38,     .Ki = 3.36,     .Kd = 0.00,
    .OutMax = 50,   .OutMin = -50,
    .ErrorIntMax = 20, .ErrorIntMin = -20,
};
#if ENABLE_POS_LOOP
/* ========== 位置环 PID ========== */
PID_t PosPID = {
    .Kp = 0.8,      .Ki = 0.0,      .Kd = 0.0,
    .OutMax = 3.0,  .OutMin = -3.0,
    .ErrorIntMax = 30, .ErrorIntMin = -30,
};
#endif

/* ========== 前馈系数 ========== */
#if ENABLE_FEEDFORWARD
#define K_FF      0.5f
#endif

/* ========== 位置累计变量 ========== */
#if ENABLE_POS_LOOP
static float pos_accum = 0.0f;
#endif

/* ========== 状态变量 ========== */
int16_t AX, AY, AZ, GX, GY, GZ;
float   AngleAcc, AngleGyro, Angle;
volatile uint8_t RunFlag = 0;

int16_t LeftPWM, RightPWM;
int16_t AvePWM, DifPWM;

volatile int8_t Debug_LV = 0;
volatile int8_t Debug_RH = 0;

float   LeftSpeed, RightSpeed;
float   AveSpeed, DifSpeed;

/* ========== 内部分频计数 ========== */
static uint8_t Count_10ms = 0;
static uint8_t Count_50ms = 0;
static volatile uint8_t Flag_Fallen = 0;

static void Task_10ms(void);
static void Task_50ms(void);

/* ============================================================ */
void Task_Init(void)
{
    PID_Init(&AnglePID);
    PID_Init(&SpeedPID);
    PID_Init(&TurnPID);

    #if ENABLE_POS_LOOP
    PID_Init(&PosPID);
    #endif
}

/* ============================================================ */
void Task_1ms_Tick(void)
{
    Key_Tick();

    if (++Count_10ms >= 10)
    {
        Count_10ms = 0;
        Task_10ms();
    }

    if (++Count_50ms >= 50)
    {
        Count_50ms = 0;
        Task_50ms();
    }
}

/* ============================================================
 * Task_10ms：姿态 + 互补滤波 + 角度环 PID
 * ============================================================ */
static void Task_10ms(void)
{
    MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);

    GY -= 35;

    AngleAcc = -atan2f(AX, AZ) / 3.14159f * 180.0f;
    AngleAcc += 5.6f;

    AngleGyro = Angle + GY / 32768.0f * 2000.0f * 0.01f;

    float Alpha = 0.01f;
    Angle = Alpha * AngleAcc + (1.0f - Alpha) * AngleGyro;

    if (Angle > 50.0f || Angle < -50.0f)
    {
        if (RunFlag == 1)
        {
            RunFlag = 0;
            Flag_Fallen = 1;
        }
    }

    if (RunFlag)
    {
        AnglePID.Actual = Angle;
        PID_Update(&AnglePID);
        AvePWM = -(int16_t)AnglePID.Out;

        LeftPWM  = AvePWM + DifPWM / 2;
        RightPWM = AvePWM - DifPWM / 2;

        if (LeftPWM  >  100) LeftPWM  =  100;
        if (LeftPWM  < -100) LeftPWM  = -100;
        if (RightPWM >  100) RightPWM =  100;
        if (RightPWM < -100) RightPWM = -100;

        Motor_SetPWM(1, LeftPWM);
        Motor_SetPWM(2, RightPWM);
    }
    else
    {
        Motor_SetPWM(1, 0);
        Motor_SetPWM(2, 0);
    }
}

/* ============================================================
 * Task_50ms：编码器 + 速度环 + 转向环
 * ============================================================ */
static void Task_50ms(void)
{
    LeftSpeed  = Encoder_Get(1) / 44.0f / 0.05f / 9.27666f;
    RightSpeed = Encoder_Get(2) / 44.0f / 0.05f / 9.27666f;

    AveSpeed = (LeftSpeed + RightSpeed) / 2.0f;
    DifSpeed = LeftSpeed - RightSpeed;

    if (RunFlag)
    {
        /* ============================================================
         * 位置环（预留，默认不启用）
         * 启用后：摇杆 → 位置环 → 速度环 → 角度环 → 电机
         * ============================================================ */
        #if ENABLE_POS_LOOP
        /* 加死区，避免静止时积分漂移 */
        if(AveSpeed  < -0.05f || AveSpeed > 0.05f)
        {
            pos_accum += AveSpeed * 0.05f;   /* 速度积分 = 位移 */
        }

        PosPID.Actual = pos_accum;
        PID_Update(&PosPID);

        SpeedPID.Target = PosPID.Out;    /* 位置环输出 → 速度环目标 */
        #endif

        /* ---- 速度环 ---- */
        SpeedPID.Actual = AveSpeed;
        PID_Update(&SpeedPID);

        /* ============================================================
        * 角度环目标 = 速度环输出（+ 可选前馈）
        * ============================================================ */
        AnglePID.Target = SpeedPID.Out;

        #if ENABLE_FEEDFORWARD
        AnglePID.Target += K_FF * SpeedPID.Target;   /* 速度前馈 */
        #endif

        /* ---- 转向环 ---- */
        TurnPID.Actual = DifSpeed;
        PID_Update(&TurnPID);
        DifPWM = (int16_t)TurnPID.Out;
    }
}

/* ============================================================
 * Task_Main
 * ============================================================ */
void Task_Main(void)
{
    if (Flag_Fallen)
    {
        Flag_Fallen = 0;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        Serial_Printf("FALLEN\r\n");
    }

    uint8_t key = Key_GetNum();
    if (key == 1)
    {
        if (RunFlag == 0)
        {
            PID_Init(&AnglePID);
            PID_Init(&SpeedPID);
            PID_Init(&TurnPID);

            #if ENABLE_POS_LOOP
            PID_Init(&PosPID);
            pos_accum = 0.0f;
            PosPID.Target = 0.0f;
            PosPID.Actual = 0.0f;
            PosPID.Actual1 = 0.0f;
            #endif
            SpeedPID.Target = 0.0f; 
            //如果SpeedPID.Target 没被清零，AnglePID.Target 用了残留的 SpeedPID.Out，会有一瞬间的冲击。

            AnglePID.Target  = 0.0f;
            AnglePID.Actual  = Angle;
            AnglePID.Actual1 = Angle;
            SpeedPID.Actual  = AveSpeed;
            SpeedPID.Actual1 = AveSpeed;
            TurnPID.Actual   = DifSpeed;
            TurnPID.Actual1  = DifSpeed;

            RunFlag = 1;
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }
        else
        {
            RunFlag = 0;
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
    }

    if (key == 2)
    {
    #if ENABLE_POS_LOOP
            if (RunFlag == 1)          /* 只在运行时有效 */
            {
                pos_accum = 0.0f;
                PosPID.Target  = 0.0f;
                PosPID.Actual  = 0.0f;
                PosPID.Actual1 = 0.0f;
                PosPID.Error_cur = 0.0f;
                PosPID.Error_last = 0.0f;
                PosPID.ErrorInt = 0.0f;    /* 顺便清积分，避免残留 */
            }
    #endif
    }

    /* ========== 蓝牙调参 ========== */
    if (BlueSerial_RxFlag == 1)
    {
        char *Tag = strtok(BlueSerial_RxPacket, ",");
        if (Tag != NULL && strcmp(Tag, "slider") == 0)
        {
            char *Name  = strtok(NULL, ",");
            char *Value = strtok(NULL, ",");
            if (Name && Value)
            {
                if      (strcmp(Name, "AngleKp") == 0) AnglePID.Kp = atof(Value);
                else if (strcmp(Name, "AngleKi") == 0) AnglePID.Ki = atof(Value);
                else if (strcmp(Name, "AngleKd") == 0) AnglePID.Kd = atof(Value);
                else if (strcmp(Name, "Offset")  == 0) AnglePID.OutOffset = atof(Value);

                else if (strcmp(Name, "SpeedKp") == 0) SpeedPID.Kp = atof(Value);
                else if (strcmp(Name, "SpeedKi") == 0) SpeedPID.Ki = atof(Value);
                else if (strcmp(Name, "SpeedKd") == 0) SpeedPID.Kd = atof(Value);

                else if (strcmp(Name, "TurnKp")  == 0) TurnPID.Kp = atof(Value);
                else if (strcmp(Name, "TurnKi")  == 0) TurnPID.Ki = atof(Value);
                else if (strcmp(Name, "TurnKd")  == 0) TurnPID.Kd = atof(Value);

                #if ENABLE_POS_LOOP
                else if (strcmp(Name, "PosKp")  == 0) PosPID.Kp = atof(Value);
                else if (strcmp(Name, "PosKi")  == 0) PosPID.Ki = atof(Value);
                else if (strcmp(Name, "PosKd")  == 0) PosPID.Kd = atof(Value);
                #endif
            }
        }
        else if (Tag != NULL && strcmp(Tag, "joystick") == 0)
        {
            int8_t LH = atoi(strtok(NULL, ","));
            int8_t LV = atoi(strtok(NULL, ","));
            int8_t RH = atoi(strtok(NULL, ","));
            int8_t RV = atoi(strtok(NULL, ","));
            (void)LH; (void)RV;

            Debug_LV = LV;
            Debug_RH = RH;

            #if ENABLE_POS_LOOP
            /* 位置环启用：摇杆写位置目标 */
            PosPID.Target  = LV * 0.08f;
            //太短改 0.15f;
            //太长改 0.04f;


            #else
            /* 速度环直控：摇杆写速度目标（当前默认） */
            SpeedPID.Target = LV / 25.0f;
            #endif

            TurnPID.Target  = RH / 25.0f;   /* 左右：转向环目标 */
        }
        BlueSerial_RxFlag = 0;
    }

    /* ========== OLED 显示 ========== */
    static uint32_t last_oled = 0;
    if (HAL_GetTick() - last_oled >= 100)
    {
        last_oled = HAL_GetTick();
        OLED_Clear();

        /* ---- 左列：角度环 ---- */
        OLED_Printf(0, 0,  OLED_6X8, "  Angle");
        OLED_Printf(0, 8,  OLED_6X8, "P:%05.2f", AnglePID.Kp);
        OLED_Printf(0, 16, OLED_6X8, "I:%05.2f", AnglePID.Ki);
        OLED_Printf(0, 24, OLED_6X8, "D:%05.2f", AnglePID.Kd);
        OLED_Printf(0, 32, OLED_6X8, "T:%+05.1f", AnglePID.Target);
        OLED_Printf(0, 40, OLED_6X8, "A:%+05.1f", Angle);
        OLED_Printf(0, 48, OLED_6X8, "O:%+05.0f", AnglePID.Out);
        OLED_Printf(0, 56, OLED_6X8, "GY:%+05d", GY);

        OLED_Printf(56, 56, OLED_6X8, "Offset:%02.0f", AnglePID.OutOffset);

        /* ---- 中列：速度环 ---- */
        OLED_Printf(50, 0,  OLED_6X8, "Speed");
        OLED_Printf(50, 8,  OLED_6X8, "%05.2f", SpeedPID.Kp);
        OLED_Printf(50, 16, OLED_6X8, "%05.2f", SpeedPID.Ki);
        OLED_Printf(50, 24, OLED_6X8, "%05.2f", SpeedPID.Kd);
        OLED_Printf(50, 32, OLED_6X8, "%+05.1f", SpeedPID.Target);
        OLED_Printf(50, 40, OLED_6X8, "%+05.1f", AveSpeed);
        OLED_Printf(50, 48, OLED_6X8, "%+05.0f", SpeedPID.Out);

        /* ---- 右列：转向环 ---- */
        OLED_Printf(88, 0,  OLED_6X8, "Turn");
        OLED_Printf(88, 8,  OLED_6X8, "%05.2f", TurnPID.Kp);
        OLED_Printf(88, 16, OLED_6X8, "%05.2f", TurnPID.Ki);
        OLED_Printf(88, 24, OLED_6X8, "%05.2f", TurnPID.Kd);
        OLED_Printf(88, 32, OLED_6X8, "%+05.1f", TurnPID.Target);
        OLED_Printf(88, 40, OLED_6X8, "%+05.1f", DifSpeed);
        OLED_Printf(88, 48, OLED_6X8, "%+05.0f", TurnPID.Out);

        OLED_Update();
    }

    /* ========== 波形打印（50ms 一次，DMA 非阻塞） ========== */
    static uint32_t last_plot = 0;
    if (HAL_GetTick() - last_plot >= 50)
    {
        last_plot = HAL_GetTick();
        BlueSerial_Printf_DMA("[plot,%f,%f]", TurnPID.Target, DifSpeed);
    }
}