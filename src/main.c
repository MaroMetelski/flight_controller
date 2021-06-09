/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0	DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)
#endif
/* A build error here means your board isn't set up to blink an LED. */

void main(void)
{
	const struct device *led1;
	const struct device *led0;
	bool led_is_on = true;
	int ret;

	led0 = device_get_binding(LED0);
	led1 = device_get_binding(LED1);
	if (led1 == NULL || led0 == NULL) {
		return;
	}

	ret = gpio_pin_configure(led0, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS0);
	ret = gpio_pin_configure(led1, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1);
	if (ret < 0) {
		return;
	}

	while (1) {
		gpio_pin_set(led0, PIN0, (int)!led_is_on);
		gpio_pin_set(led1, PIN1, (int)led_is_on);
		led_is_on = !led_is_on;
		k_msleep(1000);
	}
}
