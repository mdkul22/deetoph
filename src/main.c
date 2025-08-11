#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/led_strip.h>

#define MY_I2C      DT_ALIAS(i2c1)
#define L1X_TOP     DT_ALIAS(vl53l1x)
#define RGB         DT_ALIAS(led_strip)
int distance = 2000;

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
        return;
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
		distance = (int)sensor_value_to_double(&value);
		if (distance < 30) {
		    pixel.r = 0xf;
		    pixel.g = 0xf;
		    pixel.b = 0xf;
		} else if (distance < 100 && distance > 30) {
		    pixel.r = (int)(((float)((float)distance/50.f))*64);
		    pixel.g = 0x00;
		    pixel.b = 0x00;
		} else if (distance > 500 && distance > 100) {
		    pixel.r = 0x00;
		    pixel.g = (int)(((float)((float)distance/100.f))*64);
		    pixel.b = 0x00;
		} else {
		    pixel.r = 0x00;
		    pixel.g = 0x00;
		    pixel.b = (int)(((float)((float)distance/2000.f))*64);
		}
		ret = led_strip_update_rgb(rgb, &pixel, 1);
		if (ret) {
		    printk("Couldn't set LED\n");
		}
		printk("distance: %dmm\n", distance);
		k_sleep(K_MSEC(200));
    }
exit:
    return ret;
}

