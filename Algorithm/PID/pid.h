#ifndef PID_H
#define PID_H

typedef struct {
    float Target;
    float Actual;
    float Actual1;
    float Out;

    float Kp;
    float Ki;
    float Kd;

    float Error_cur;
    float Error_last;
    float ErrorInt;

    float ErrorIntMax;
    float ErrorIntMin;

    float OutMax;
    float OutMin;

    float OutOffset;
} PID_t;

void PID_Init(PID_t *p);
void PID_Update(PID_t *p);

#endif