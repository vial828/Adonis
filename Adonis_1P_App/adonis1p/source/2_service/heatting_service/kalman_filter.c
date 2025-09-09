// kalman_filter.c

#include "kalman_filter.h"

 

void Kalman_Init(KalmanFilter *kf, float init_x, float init_p, float q, float r) {

    kf->x = init_x;

    kf->p = init_p;

    kf->q = q;

    kf->r = r;

}

 

float Kalman_Update(KalmanFilter *kf, float z) {

    // Prediction update

    kf->p += kf->q;

 

    // Measurement update

    kf->k = kf->p / (kf->p + kf->r);

    kf->x += kf->k * (z - kf->x);

    kf->p *= (1 - kf->k);

 

    return kf->x;

}
/* example on how to use this filter
int main() {

    KalmanFilter kf;

    Kalman_Init(&kf, 7.342, 1.000, 0.001, 1.000);

 

    float adc_data[] = {7.342, 7.286, 7.229, 7.173, 7.117, 7.061, 7.004, 6.948, 6.892, 6.835, 6.779, 6.743, 6.706, 6.669, 6.633, 6.596, 6.559, 6.523, 6.486, 6.449, 6.412, 6.355, 6.298, 6.241, 6.184, 6.127, 6.070, 6.013, 5.956, 5.899, 5.843, 5.806, 5.769, 5.732, 5.696, 5.659, 5.623, 5.586, 5.550, 5.514, 5.477, 5.421, 5.365, 5.309, 5.252, 5.196, 5.140, 5.084, 5.027, 4.971}; // First 50 samples

    int data_len = sizeof(adc_data) / sizeof(adc_data[0]);

 

    for (int i = 0; i < data_len; i++) {

        float filtered = Kalman_Update(&kf, adc_data[i]);

        printf("Raw: %.3f -> Filtered: %.3f\n", adc_data[i], filtered);

    }

 

    return 0;

}
*/

