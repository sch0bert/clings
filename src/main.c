/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include "lis2dtw12.h"

#define DT_DRV_COMPAT st_lis2dtw12
#define SLEEP_TIME_MS   1000

LOG_MODULE_REGISTER(Main, CONFIG_SENSOR_LOG_LEVEL);

void main(void)
{
	LOG_INF("LIS2DTW12 accelerometer sensor");
	if (!device_is_ready(dev_i2c.bus)) {
		LOG_ERR("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
		return;
	}

	int ret = lis2dtw12_init(dev_i2c.bus);
	if (ret < 0) {
		LOG_ERR("Failed to initialize i2c interface for LIS2DTW12, error: %d\n\r", ret);
		return;
	}

	while (1) {

		k_msleep(SLEEP_TIME_MS);
	}
}
