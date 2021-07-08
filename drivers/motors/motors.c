#define DT_DRV_COMPAT motors

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include "drivers/motors.h"

#define LOG_LEVEL CONFIG_MOTORS_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(motors);

#define MOTORS_NUM 4

#if DT_INST_PROP_LEN(0, pwms) != MOTORS_NUM
#error Unsupported configuration! Motors module requires 4 pwms!
#endif

struct motor_pwm {
	const struct device *dev;
	uint32_t channel;
	uint32_t period;
	pwm_flags_t flags;
};

int set_throttle(const struct device *dev, uint32_t motor, uint32_t throttle)
{


	return 0;
}

static int motors_init(const struct device *dev)
{
	LOG_INF("Motors module initialized");

	return 0;
}

#define PWM_NODE(idx) DT_INST_PWMS_CTLR_BY_IDX(0, idx)

#define CONFIGURE_MOTOR(nodeid, prop, idx)		\
{ \
	.dev = DEVICE_DT_GET(PWM_NODE(idx)),		\
	.channel = DT_INST_PWMS_CHANNEL_BY_IDX(0, idx),	\
	.period = DT_INST_PWMS_PERIOD_BY_IDX(0, idx),	\
	.flags = DT_INST_PWMS_FLAGS_BY_IDX(0, idx),		\
},						\

struct motor_pwm motors[MOTORS_NUM] = {
	DT_INST_FOREACH_PROP_ELEM(0, pwms, CONFIGURE_MOTOR)
};

DEVICE_DEFINE(motors, "motors", motors_init, NULL, NULL, NULL, POST_KERNEL, 99, NULL);
