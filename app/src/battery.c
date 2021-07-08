#include <logging/log.h>
#include <drivers/adc.h>
#include <device.h>
#include "battery.h"

LOG_MODULE_REGISTER(battery, LOG_LEVEL_INF);

#define ADC_RESOLUTION 12
#define ADC_GAIN ADC_GAIN_1
#define ADC_LABEL DT_LABEL(DT_NODELABEL(adc1))
#define VREF 3300 //voltage reference is Vdd

static int16_t sample_buffer[1];

static struct adc_channel_cfg channel_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id = 0,
	.differential = 0
};

static struct adc_sequence sequence = {
	.channels = BIT(0),
	.buffer = sample_buffer,
	.buffer_size = sizeof(sample_buffer),
	.resolution = ADC_RESOLUTION,
};

static int battery_init(const struct device *dev)
{
	dev = device_get_binding(ADC_LABEL);
	adc_channel_setup(dev, &channel_cfg);

	LOG_INF("Initializing battery module...");
	return 0;
}

int32_t battery_read(void)
{
	const struct device *dev = device_get_binding(ADC_LABEL);
	if (!device_is_ready(dev)) {
		LOG_ERR("ADC not ready!");
		return -1;
	}
	int err = adc_read(dev, &sequence);
	if (err != 0) {
		LOG_ERR("Couldn't read battery voltage!");
		return err;
	}
	int32_t raw_value = sample_buffer[0];
	LOG_DBG("ADC raw value: %i", raw_value);
	int32_t value_mv = raw_value;
	adc_raw_to_millivolts(VREF, ADC_GAIN, ADC_RESOLUTION, &value_mv);
	return value_mv;
}

SYS_INIT(battery_init, APPLICATION, 98);
