// kalman_filter.h

#ifndef KALMAN_FILTER_H

#define KALMAN_FILTER_H

 

typedef struct {

    float x;      // Estimated value

    float p;      // Estimation error covariance

    float q;      // Process noise covariance

    float r;      // Measurement noise covariance

    float k;      // Kalman gain

} KalmanFilter;

 

void Kalman_Init(KalmanFilter *kf, float init_x, float init_p, float q, float r);

float Kalman_Update(KalmanFilter *kf, float z);

 

#endif // KALMAN_FILTER_H

