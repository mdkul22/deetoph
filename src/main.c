#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/led_strip.h>

#define MY_I2C      DT_ALIAS(i2c1)
#define L1X_TOP     DT_ALIAS(vl53l1x)
#define RGB         DT_ALIAS(led_strip)

int main(void)
{
    const struct device *const i2c_dev = DEVICE_DT_GET(MY_I2C);
    const struct device *const l1x_top = DEVICE_DT_GET(L1X_TOP);
    const struct device *const rgb = DEVICE_DT_GET(RGB);

    struct led_rgb pixel = {0, 0, 0};
    int ret = 0;
	struct sensor_value value;
    if (!device_is_ready(i2c_dev)) {
        printk("I2C device not ready!\n");
        ret = -ENODEV;
        goto exit;
    }
    if (!device_is_ready(l1x_top)) {
        printk("vl53l1x not ready!\n");
        ret = -ENODEV;
        goto exit;
    }
    if (!device_is_ready(rgb)) {
        printk("LED not ready!\n");
        return -ENODEV;
    }
    printk("Prototyper begin!\n");
    for (;;) {
		ret = sensor_sample_fetch(l1x_top);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			ret = -ENODATA;
			goto exit;
		}

		ret = sensor_channel_get(l1x_top,
					 SENSOR_CHAN_DISTANCE,
					 &value);
		double distance = sensor_value_to_double(&value);
		if (distance < 30) {
		    pixel.r = 0xf;
		    pixel.g = 0xf;
		    pixel.b = 0xf;
		} else if (distance < 100 && distance > 30) {
		    pixel.r = (uint8_t)((distance / 100.0) * 32.0);
		    pixel.g = 0x00;
		    pixel.b = 0x00;
		} else if (distance > 500 && distance > 100) {
		    pixel.r = 0x00;
		    pixel.g = (uint8_t)((distance / 500.0) * 32.0);
		    pixel.b = 0x00;
		} else {
		    pixel.r = 0x00;
		    pixel.g = 0x00;
		    pixel.b = (uint8_t)((distance / 2000.0) * 32.0);
		}
		ret = led_strip_update_rgb(rgb, &pixel, 1);
		if (ret) {
		    printk("Couldn't set LED\n");
		}
		printk("distance: %dmm\n", (int)distance);
		k_sleep(K_MSEC(50));
    }
exit:
    return ret;
}

