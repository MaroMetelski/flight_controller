#ifndef __IMU_H__
#define __IMU_H__

#include <arm_math.h>

struct cartesian {
	float32_t x;
	float32_t y;
	float32_t z;
};

struct euler_angles {
	float32_t pitch;
	float32_t roll;
	float32_t yaw;

};

struct imu_data {
	struct cartesian accel;
	struct cartesian gyro;
	float temperature;
};

void imu_get_data(struct imu_data *data);
void imu_get_position(struct euler_angles *data);
void imu_get_quaternion(float32_t *q);
#endif
