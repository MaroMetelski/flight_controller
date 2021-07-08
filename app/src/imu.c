#include <logging/log.h>
#include <init.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>

#include <math.h>
#include <arm_math.h>
#include "imu.h"

LOG_MODULE_REGISTER(imu, LOG_LEVEL_DBG);

#define IMU DT_LABEL(DT_INST(0, invensense_mpu6050))

K_MUTEX_DEFINE(imu_lock);

static struct raw_imu_data {
	struct sensor_value accel_xyz[3];
	struct sensor_value gyro_xyz[3];
	struct sensor_value temperature;
} current_raw_imu_data;

static struct imu_data current_imu_data = {
	.accel = {0},
	.gyro = {0},
	.temperature = 0,
};

static struct imu_data errors = {
	.accel = {0},
	.gyro = {0},
	.temperature = 0,
};

static struct euler_angles current_position = {
	.pitch = 0,
	.roll = 0,
	.yaw = 0,
};


static void quaternion2euler(float32_t *q, struct euler_angles *eu)
{
	/**
	 * https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	 */

	 float32_t one = 2 * (q[0] * q[1] + q[2] * q[3]);
	 float32_t two = 1 - 2 * (q[2] * q[2]  - q[3] * q[3]);
	 eu->roll = atan2(one, two);

	 float32_t sinp = 2 * (q[0] * q[2] - q[3] * q[1]);
	 if (fabs(sinp) >= 1.0)
		 eu->pitch = copysign(SENSOR_PI / 2, sinp);
	 else
		 eu->pitch = asin(sinp);
	 float32_t three = 2 * (q[0] * q[3] + q[1] * q[2]);
	 float32_t four = 1 - 2 * (q[1] * q[1] - q[3] * q[3]);
	 eu->yaw = atan2(three, four);

}

static float32_t quaternion_current[4] = {1, 0, 0, 0};

static void calculate_current_position(float32_t *q_pos,
				       struct imu_data *data,
				       struct imu_data *errors,
				       int64_t dt)
{

	// derivative quaternion
	float32_t q_dt[4] = {1, 0, 0, 0};
	// angular velocity quaternion
	float32_t q_v[4];
	q_v[0] = 0.0;
	q_v[1] = data->gyro.x - errors->gyro.x;
	q_v[2] = data->gyro.y - errors->gyro.y;
	q_v[3] = data->gyro.z - errors->gyro.z;

	// derivative of quaternion
	// right side `v` when `v` is in body reference frame
	// q_dt = 0.5 * q_pos x q_v
	arm_quaternion_product_single_f32(q_pos, q_v, q_dt);
	for (int i = 0; i < 4; i++) {
		q_dt[i] *= 0.5;
	}

	// integrate, simple euler method
	for (int i = 0; i < 4; i++) {
		q_pos[i] = q_pos[i] + (q_dt[i] * (dt / 1000.0)); // dt ms to s
	}

	arm_quaternion_normalize_f32(q_pos, q_pos, 1);
}

static int imu_read_data(const struct device *dev)
{
	int ret;
	ret = sensor_sample_fetch(dev);

	if (ret == 0) {
		ret = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ,
					 current_raw_imu_data.accel_xyz);
	}
	if (ret == 0) {
		ret = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ,
					 current_raw_imu_data.gyro_xyz);
	}
	if (ret == 0) {
		ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP,
					 &current_raw_imu_data.temperature);
	}
	return ret;
}

static void imu_parse_data(struct imu_data *target,
			   struct raw_imu_data *raw)
{
	target->accel.x = sensor_value_to_double(&raw->accel_xyz[0]);
	target->accel.y = sensor_value_to_double(&raw->accel_xyz[1]);
	target->accel.z = sensor_value_to_double(&raw->accel_xyz[2]);
	target->gyro.x = sensor_value_to_double(&raw->gyro_xyz[0]);
	target->gyro.y = sensor_value_to_double(&raw->gyro_xyz[1]);
	target->gyro.z = sensor_value_to_double(&raw->gyro_xyz[2]);
	target->temperature = sensor_value_to_double(&raw->temperature);

}

K_THREAD_STACK_DEFINE(imu_stack, 2048);
static struct k_thread imu_thread;

static void imu_thread_start(void *u1, void *u2, void *u3)
{
	LOG_INF("IMU thread starting...");
	const struct device *dev = device_get_binding(IMU);
	if (!dev) {
		LOG_ERR("Couldn't initialize IMU device");
		return;
	}
	int64_t timestamp = 0;
	int64_t dt = 0;

	float gyro_sumx = 0;
	float gyro_sumy = 0;
	float gyro_sumz = 0;
	int err_samples = 500;
	/* get error values */
	for (int i = 0; i < err_samples; i++)
	{
		k_mutex_lock(&imu_lock, K_FOREVER);
		imu_read_data(dev);
		imu_parse_data(&current_imu_data,
			       &current_raw_imu_data);
		k_mutex_unlock(&imu_lock);
		gyro_sumx += current_imu_data.gyro.x;
		gyro_sumy += current_imu_data.gyro.y;
		gyro_sumz += current_imu_data.gyro.z;
		k_sleep(K_MSEC(10));
	}

	errors.gyro.x = gyro_sumx / err_samples;
	errors.gyro.y = gyro_sumy / err_samples;
	errors.gyro.z = gyro_sumz / err_samples;

	char msg[100];
	sprintf(msg, "error: %f, %f, %f",
		errors.gyro.x,
		errors.gyro.y,
		errors.gyro.z);
	LOG_INF("%s", log_strdup(msg));


	while (true) {
		k_sleep(K_MSEC(10));
		dt = k_uptime_delta(&timestamp);
		imu_read_data(dev);
		k_mutex_lock(&imu_lock, K_FOREVER);
		imu_parse_data(&current_imu_data,
			       &current_raw_imu_data);
		k_mutex_unlock(&imu_lock);
		calculate_current_position(quaternion_current,
					   &current_imu_data,
					   &errors,
					   dt);
		k_mutex_lock(&imu_lock, K_FOREVER);
		quaternion2euler(quaternion_current,
				 &current_position);
		k_mutex_unlock(&imu_lock);
		timestamp = k_uptime_get();
		/* LOG_INF("loop complete, ms=%d", (int)dt); */
	}
}

static int imu_init(const struct device *dev)
{
	k_thread_create(&imu_thread, imu_stack,
			K_THREAD_STACK_SIZEOF(imu_stack),
			imu_thread_start, NULL, NULL, NULL,
			40, 0, K_NO_WAIT);
	k_thread_name_set(&imu_thread, "imu");
	return 0;
}

void imu_get_position(struct euler_angles* data)
{
	k_mutex_lock(&imu_lock, K_FOREVER);
	data->pitch = current_position.pitch;
	data->roll = current_position.roll;
	data->yaw = current_position.yaw;
	k_mutex_unlock(&imu_lock);

}
void imu_get_data(struct imu_data* data)
{
	k_mutex_lock(&imu_lock, K_FOREVER);
	data->gyro = current_imu_data.gyro;
	data->accel = current_imu_data.accel;
	data->temperature = current_imu_data.temperature;
	k_mutex_unlock(&imu_lock);
}

void imu_get_quaternion(float32_t *q)
{
	for (int i = 0; i < 4; i++) {
		q[i] = quaternion_current[i];
	}
}

SYS_INIT(imu_init, APPLICATION, 98);
