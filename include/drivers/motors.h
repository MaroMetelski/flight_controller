#ifndef __MOTORS_H__
#define __MOTORS_H__

#include <zephyr.h>
#include <device.h>

int motors_set_throttle(const struct device *dev,
			uint32_t motor, uint32_t throttle);

#endif // __MOTORS_H__
