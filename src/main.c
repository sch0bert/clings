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

/* Positions */
#define LIS2DW12_6D_PORT_RIGHT		0x08
#define LIS2DW12_6D_PORT_LEFT		0x04
#define LIS2DW12_6D_FACE_UP			0x02
#define LIS2DW12_6D_FACE_DOWN		0x01
#define LIS2DW12_6D_STANDING		0x20
#define LIS2DW12_6D_UPSIDE_DOWN		0x10

LOG_MODULE_REGISTER(Main, CONFIG_SENSOR_LOG_LEVEL);

static void fetch_and_display(const struct device *sensor)
{
	struct sensor_value temperature;
	int rc = sensor_sample_fetch(sensor); // Dummy read to trigger a sample

	if (rc == 0) {
		rc = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
		if (rc < 0) {
			LOG_ERR("ERROR: Unable to read temperature:%d", rc);
		} else {
			LOG_INF("%.02f C", sensor_value_to_double(&temperature));
		}
	}	
}

static void trigger_handler(const struct device *dev,
			    const struct sensor_trigger *trig)
{
	struct sensor_value orientation;
	int rc = sensor_sample_fetch(dev);

	if(rc == 0) {
		rc = sensor_channel_get(dev, SENSOR_CHAN_ROTATION, &orientation);
		if (rc < 0) {
			LOG_ERR("ERROR: Unable to read orientation:%d", rc);
		} else {
			uint16_t rotation = sensor_value_to_double(&orientation);

			switch(rotation) {
				case LIS2DW12_6D_PORT_RIGHT:
					LOG_INF("Portrait right");
					break;
				case LIS2DW12_6D_PORT_LEFT:
					LOG_INF("Portrait left");
					break;
				case LIS2DW12_6D_FACE_UP:
					LOG_INF("Standing");
					break;
				case LIS2DW12_6D_FACE_DOWN:
					LOG_INF("Upside down");
					break;
				case LIS2DW12_6D_STANDING:
					LOG_INF("Face down");
					break;
				case LIS2DW12_6D_UPSIDE_DOWN:
					LOG_INF("Face up");
					break;
				default:
					LOG_INF("Unknown - rotation: %d", rotation);
					break;
			}
		}
	}
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

