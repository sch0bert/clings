/* ST Microelectronics LIS2DTW12 3-axis accelerometer driver
 *
 * Copyright (c) 2019 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Datasheet:
 * https://www.st.com/resource/en/datasheet/lis2dtw12.pdf
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_LIS2DTW12_LIS2DTW12_H_
#define ZEPHYR_DRIVERS_SENSOR_LIS2DTW12_LIS2DTW12_H_

#if CONFIG_SPI
#include <zephyr/drivers/spi.h>
#elif CONFIG_I2C
#include <zephyr/drivers/i2c.h>
#endif
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/sensor.h>
#include "lis2dtw12_reg.h"

#define LIS2DTW12_SLAVE_ADDRESS 0x18
#define I2C1_NODE DT_NODELABEL(st_lis2dtw12)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);

union axis3bit16_t {
	int16_t i16bit[3];
	uint8_t u8bit[6];
};

#if defined(CONFIG_LIS2DTW12_ODR_1_6)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_1Hz6_LP_ONLY
#elif defined(CONFIG_LIS2DTW12_ODR_12_5)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_12Hz5
#elif defined(CONFIG_LIS2DTW12_ODR_25)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_25Hz
#elif defined(CONFIG_LIS2DTW12_ODR_50)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_50Hz
#elif defined(CONFIG_LIS2DTW12_ODR_100) || \
	defined(CONFIG_LIS2DTW12_ODR_RUNTIME)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_100Hz
#elif defined(CONFIG_LIS2DTW12_ODR_200)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_200Hz
#elif defined(CONFIG_LIS2DTW12_ODR_400)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_400Hz
#elif defined(CONFIG_LIS2DTW12_ODR_800)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_800Hz
#elif defined(CONFIG_LIS2DTW12_ODR_1600)
	#define LIS2DTW12_DEFAULT_ODR	LIS2DTW12_XL_ODR_1k6Hz
#endif

/* Return ODR reg value based on data rate set */
#define LIS2DTW12_ODR_TO_REG(_odr) \
	((_odr <= 1) ? LIS2DTW12_XL_ODR_1Hz6_LP_ONLY : \
	 (_odr <= 12) ? LIS2DTW12_XL_ODR_12Hz5 : \
	 ((31 - __builtin_clz(_odr / 25))) + 3)

/* FS reg value from Full Scale */
#define LIS2DTW12_FS_TO_REG(_fs)	(30 - __builtin_clz(_fs))

#if defined(CONFIG_LIS2DTW12_ACCEL_RANGE_RUNTIME) || \
	defined(CONFIG_LIS2DTW12_ACCEL_RANGE_2G)
	#define LIS2DTW12_ACC_FS		LIS2DTW12_2g
#elif defined(CONFIG_LIS2DTW12_ACCEL_RANGE_4G)
	#define LIS2DTW12_ACC_FS		LIS2DTW12_4g
#elif defined(CONFIG_LIS2DTW12_ACCEL_RANGE_8G)
	#define LIS2DTW12_ACC_FS		LIS2DTW12_8g
#elif defined(CONFIG_LIS2DTW12_ACCEL_RANGE_16G)
	#define LIS2DTW12_ACC_FS		LIS2DTW12_16g
#endif

/* Acc Gain value in ug/LSB in High Perf mode */
#define LIS2DTW12_FS_2G_GAIN		244
#define LIS2DTW12_FS_4G_GAIN		488
#define LIS2DTW12_FS_8G_GAIN		976
#define LIS2DTW12_FS_16G_GAIN		1952

#define LIS2DTW12_SHFT_GAIN_NOLP1	2
#define LIS2DTW12_ACCEL_GAIN_DEFAULT_VAL LIS2DTW12_FS_2G_GAIN
#define LIS2DTW12_FS_TO_GAIN(_fs, _lp1) \
		(LIS2DTW12_FS_2G_GAIN << ((_fs) + (_lp1)))

/* shift value for power mode */
#define LIS2DTW12_SHIFT_PM1		4
#define LIS2DTW12_SHIFT_PMOTHER		2

/**
 * struct lis2dtw12_device_config - lis2dtw12 hw configuration
 * @bus_name: Pointer to bus master identifier.
 * @pm: Power mode (lis2dh_powermode).
 * @int_gpio_port: Pointer to GPIO PORT identifier.
 * @int_gpio_pin: GPIO pin number connecter to sensor int pin.
 * @int_pin: Sensor int pin (int1/int2).
 */
struct lis2dtw12_device_config {
	const struct device *bus;
	lis2dtw12_mode_t pm;
#ifdef CONFIG_LIS2DTW12_TRIGGER
	const char *int_gpio_port;
	uint8_t int_gpio_pin;
	uint8_t int_gpio_flags;
	uint8_t int_pin;
#ifdef CONFIG_LIS2DTW12_PULSE
	uint8_t pulse_trigger;
	uint8_t pulse_ths[3];
	uint8_t pulse_shock;
	uint8_t pulse_ltncy;
	uint8_t pulse_quiet;
#endif /* CONFIG_LIS2DTW12_PULSE */
#endif /* CONFIG_LIS2DTW12_TRIGGER */
};

/* sensor data forward declaration (member definition is below) */
struct lis2dtw12_data;

/* sensor data */
struct lis2dtw12_data {
	const struct device *bus;
	int16_t acc[3];
	int16_t temp;

	 /* save sensitivity */
	uint16_t gain;

	stmdev_ctx_t *ctx;
#ifdef CONFIG_LIS2DTW12_TRIGGER
	const struct device *dev;
	const struct device *gpio;
	uint8_t gpio_pin;
	struct gpio_callback gpio_cb;
	sensor_trigger_handler_t drdy_handler;
#ifdef CONFIG_LIS2DTW12_PULSE
	sensor_trigger_handler_t tap_handler;
	sensor_trigger_handler_t double_tap_handler;
#endif /* CONFIG_LIS2DTW12_PULSE */
#if defined(CONFIG_LIS2DTW12_TRIGGER_OWN_THREAD)
	K_KERNEL_STACK_MEMBER(thread_stack, CONFIG_LIS2DTW12_THREAD_STACK_SIZE);
	struct k_thread thread;
	struct k_sem gpio_sem;
#elif defined(CONFIG_LIS2DTW12_TRIGGER_GLOBAL_THREAD)
	struct k_work work;
#endif /* CONFIG_LIS2DTW12_TRIGGER_GLOBAL_THREAD */
#endif /* CONFIG_LIS2DTW12_TRIGGER */
#if DT_INST_SPI_DEV_HAS_CS_GPIOS(0)
	struct spi_cs_control cs_ctrl;
#endif
};

int lis2dtw12_i2c_init(const struct device *dev);
int lis2dtw12_init(const struct device *dev);

#ifdef CONFIG_LIS2DTW12_TRIGGER
int lis2dtw12_init_interrupt(const struct device *dev);
int lis2dtw12_trigger_set(const struct device *dev,
			  const struct sensor_trigger *trig,
			  sensor_trigger_handler_t handler);
#endif /* CONFIG_LIS2DTW12_TRIGGER */

#endif /* ZEPHYR_DRIVERS_SENSOR_LIS2DTW12_LIS2DTW12_H_ */