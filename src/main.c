/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

#define SLEEP_TIME_MS   1000

LOG_MODULE_REGISTER(Main, CONFIG_SENSOR_LOG_LEVEL);

static void fetch_and_display(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	struct sensor_value temperature;
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DW12_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		LOG_ERR("ERROR: Update failed: %d", rc);
	} else {
		LOG_INF("#%u @ %u ms: %sx %f , y %f , z %f",
		       count, k_uptime_get_32(), overrun,
		       sensor_value_to_double(&accel[0]),
		       sensor_value_to_double(&accel[1]),
		       sensor_value_to_double(&accel[2]));
	}

	if (rc == 0) {
		rc = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
		if (rc < 0) {
			LOG_ERR("ERROR: Unable to read temperature:%d", rc);
		} else {
			LOG_INF(", t %f", sensor_value_to_double(&temperature));
		}
	}	
}

static void trigger_handler(const struct device *dev,
			    const struct sensor_trigger *trig)
{
	LOG_INF("Trigger fired");
}

int main(void)
{
	const struct device *const sensor = DEVICE_DT_GET_ANY(st_lis2dw12);
	static struct sensor_trigger sensor_trig;

	if (sensor == NULL) {
		LOG_ERR("No device found");
		return 0;
	}
	if (!device_is_ready(sensor)) {
		LOG_ERR("Device %s is not ready", sensor->name);
		return 0;
	}

#ifdef CONFIG_LIS2DW12_TRIGGER
	int rc = 0;

	if (rc == 0) {
		sensor_trig.type = SENSOR_TRIG_DATA_READY;
		sensor_trig.chan = SENSOR_CHAN_AMBIENT_TEMP;
		rc = sensor_trigger_set(sensor, &sensor_trig, trigger_handler);
	}

	if (rc != 0) {
		LOG_ERR("Trigger set failed: %d", rc);
		return 0;
	}
	LOG_INF("Trigger set got %d", rc);
#endif

	while (true) {
		fetch_and_display(sensor);
		k_msleep(SLEEP_TIME_MS);
	}
}

