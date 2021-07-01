#define DT_DRV_COMPAT motors

#include <zephyr.h>
#include <device.h>

#define LOG_LEVEL CONFIG_MOTORS_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(motors);



static int motors_init(const struct device *dev)
{
	LOG_INF("Motors module initialized");
	return 0;

}

DEVICE_DEFINE(motors, "motors", motors_init, NULL, NULL, NULL, POST_KERNEL, 99, NULL);
