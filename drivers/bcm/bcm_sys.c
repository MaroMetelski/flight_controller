#define DT_DRV_COMPAT sys_bcm

#include <arm_math.h>
#include <kernel.h>
#include <drivers/pwm.h>
#include <device.h>
#include <drivers/gpio.h>
#include <devicetree.h>
#include <logging/log.h>

#include "drivers/bcm_sys.h"

LOG_MODULE_REGISTER(bcm_sys, CONFIG_BCM_LOG_LEVEL);

// 10 bits resolution
#define PWM_BCM_SYS_RESOLUTION 9
#define TICKS_NUM PWM_BCM_SYS_RESOLUTION

uint32_t tick_map[TICKS_NUM];

struct bcm_sys_config {
	uint32_t period;
	// points in period in which callback is called
};

struct bcm_sys_config bcm_sys_config = {0};

struct bcm_sys_data {
	const struct device *dev;
};


static void fill_tick_map(struct bcm_sys_config *cfg, uint32_t *tick_map)
{
	for (int i = 0; i < TICKS_NUM; i++) {
		tick_map[TICKS_NUM - 1 - i] = (int)(cfg->period / (1 << (i + 1)));
	}
}

const struct gpio_dt_spec gpio0 = GPIO_DT_SPEC_GET_BY_IDX(DT_DRV_INST(0), out_gpios, 0);


int bcm_sys_set_period(uint32_t period)
{
	bcm_sys_config.period = period;
	fill_tick_map(&bcm_sys_config, tick_map);
	return 0;
}

int bcm_sys_set_pin(uint32_t pin, uint32_t duty_cycle)
{
	return 0;
}

int bcm_sys_start(void)
{
	return 0;
}

uint16_t dc = 900;

static void bcm_tick_cb(uint32_t tick_num)
{
	bool to_set = false;

	if (dc & (1 << tick_num)) {
		to_set = true;
	}
	gpio_pin_set(gpio0.port, gpio0.pin, to_set ? 1 : 0);
}

static void bcm_poll_sys_clk(void *u1, void *u2, void *u3)
{
	uint32_t diff = 0;
	uint32_t start;
	uint32_t tick_ptr = 0;

	while (true) {
		bcm_tick_cb(tick_ptr);

		start = k_cycle_get_32();
		diff = 0;
		while (diff < tick_map[tick_ptr]) {
			diff = k_cycle_get_32() - start;
		}
		tick_ptr++;
		if (tick_ptr == TICKS_NUM) {
			tick_ptr = 0;
		}
	}
}

K_THREAD_STACK_DEFINE(bcm_stack, 512);
static struct k_thread bcm_thread;

static int bcm_sys_init(const struct device *dev)
{
	LOG_INF("BCM PWM driver starting...");


	gpio_pin_configure(gpio0.port, gpio0.pin, GPIO_OUTPUT);
	bcm_sys_set_period(100000);
	k_thread_create(&bcm_thread, bcm_stack,
			K_THREAD_STACK_SIZEOF(bcm_stack),
			bcm_poll_sys_clk, NULL, NULL, NULL,
			51, 0, K_NO_WAIT);
	k_thread_name_set(&bcm_thread, "bcm_poll");
	return 0;
}

DEVICE_DEFINE(bcm_sys, "bcm_sys",
	      bcm_sys_init, NULL,
	      NULL, &bcm_sys_config,
	      POST_KERNEL, 90, NULL);
