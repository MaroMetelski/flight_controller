#include <device.h>
#include <init.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdio.h>

#include "battery.h"
#include "imu.h"

LOG_MODULE_REGISTER(controller, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(controller_stack, 2048);
static struct k_thread controller_thread;

struct euler_angles position;

static void controller_thread_start(void *u1, void *u2, void *u3)
{

	LOG_INF("Controller starting...");
	int32_t value;
	char message[100];


	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(bcm_sys0));

	if (!dev) {
		LOG_ERR("Couldn't initialize BCM");
	}
	while (true) {
		value = battery_read();
		LOG_INF("divider voltage: %d", value);

		imu_get_position(&position);
		position.pitch *= 180.0 / PI;
		position.roll *= 180.0 / PI;
		position.yaw *= 180.0 / PI;

		sprintf(message, "position: %.2f %.2f %.2f\n",
			position.pitch,
			position.roll,
			position.yaw);
		LOG_INF("%s", log_strdup(message));
		k_sleep(K_MSEC(1000));
	}
}


static int controller_init(const struct device *dev)
{
	k_thread_create(&controller_thread, controller_stack,
			K_THREAD_STACK_SIZEOF(controller_stack),
			controller_thread_start, NULL, NULL, NULL,
			50, 0, K_NO_WAIT);
	k_thread_name_set(&controller_thread, "controller");
	return 0;
}

SYS_INIT(controller_init, APPLICATION, 99);
