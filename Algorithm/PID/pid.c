#include "pid.h"

void PID_Init(PID_t *p)
{
    p->Target     = 0.0f;
    p->Actual     = 0.0f;
    p->Actual1    = 0.0f;
    p->Out        = 0.0f;
    p->Error_last = 0.0f;
    p->Error_cur  = 0.0f;
    p->ErrorInt   = 0.0f;
}

void PID_Update(PID_t *p)
{
    /* 上次误差与本次误差 */
    p->Error_last = p->Error_cur;
    p->Error_cur  = p->Target - p->Actual;

    /* 比例项 + 微分先行（避免 Target 突变带来微分冲击） */
    float out_pd = p->Kp * p->Error_cur
                 - p->Kd * (p->Actual - p->Actual1);

    /* 积分项：条件积分，输出饱和时停止累积 */
    if (p->Ki != 0.0f)
    {
        float out_test = out_pd + p->Ki * (p->ErrorInt + p->Error_cur);

        if (out_test < p->OutMax && out_test > p->OutMin)
        {
            p->ErrorInt += p->Error_cur;

            if (p->ErrorInt > p->ErrorIntMax) p->ErrorInt = p->ErrorIntMax;
            if (p->ErrorInt < p->ErrorIntMin) p->ErrorInt = p->ErrorIntMin;
        }
    }
    else
    {
        p->ErrorInt = 0.0f;
    }

    /* PID 输出 */
    p->Out = out_pd + p->Ki * p->ErrorInt;

    /* 输出偏移 */
    if (p->Out > 0.0f)
    {
        p->Out += p->OutOffset;
    }
    else if (p->Out < 0.0f)
    {
        p->Out -= p->OutOffset;
    }

    /* 输出限幅 */
    if (p->Out > p->OutMax) p->Out = p->OutMax;
    if (p->Out < p->OutMin) p->Out = p->OutMin;

    /* 保存上次实际值 */
    p->Actual1 = p->Actual;
}