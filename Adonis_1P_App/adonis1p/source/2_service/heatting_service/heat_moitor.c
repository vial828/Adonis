#include "heat_monitor.h"

KalmanFilter kf;

void Kalman_Init( float init_x, float init_p, float q, float r) {

    kf.x = init_x;

    kf.p = init_p;

    kf.q = q;

    kf.r = r;

}

 

float Kalman_Update(float z) {

    // Prediction update

    kf.p += kf.q;

 

    // Measurement update

    kf.k = kf.p / (kf.p + kf.r);

    kf.x += kf.k * (z - kf.x);

    kf.p *= (1 - kf.k);

 

    return kf.x;

}

void heat_monitor(void)
{
}

