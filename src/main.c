#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/printk.h>

#define MY_I2C      DT_ALIAS(i2c1)
#define L1X_TOP     DT_ALIAS(vl53l1x)


int main(void)
{
    const struct device *const i2c_dev = DEVICE_DT_GET(MY_I2C);
    const struct device *const l1x_top = DEVICE_DT_GET(L1X_TOP);
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
		printk("distance: %dmm\n", (int)sensor_value_to_double(&value));

		k_sleep(K_MSEC(100));
    }
exit:
    return ret;
}

