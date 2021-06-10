#include <device.h>
#include <init.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdio.h>

LOG_MODULE_REGISTER(controller, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(controller_stack, 1024);
static struct k_thread controller_thread;

static void controller_thread_start(void *u1, void *u2, void *u3)
{

	LOG_INF("Controller starting...");

	while (true) {
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
