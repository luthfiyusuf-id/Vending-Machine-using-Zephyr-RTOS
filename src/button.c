#include "button.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#define BUTTON_DEBOUNCE_MS 50

#define COIN500_NODE DT_ALIAS(coin_500)
#define COIN1000_NODE DT_ALIAS(coin_1000)
#define COIN2000_NODE DT_ALIAS(coin_2000)
#define PRODUCT_NODE DT_ALIAS(product_button)

static const struct gpio_dt_spec coin500 =
    GPIO_DT_SPEC_GET(COIN500_NODE, gpios);

static const struct gpio_dt_spec coin1000 =
    GPIO_DT_SPEC_GET(COIN1000_NODE, gpios);

static const struct gpio_dt_spec coin2000 =
    GPIO_DT_SPEC_GET(COIN2000_NODE, gpios);

static const struct gpio_dt_spec product =
    GPIO_DT_SPEC_GET(PRODUCT_NODE, gpios);

static const struct gpio_dt_spec *buttons[] = {
    &coin500,
    &coin1000,
    &coin2000,
    &product
};

int button_init(void)
{
    for (int i = 0; i < 4; i++) {

        if (!gpio_is_ready_dt(buttons[i])) {
            return -ENODEV;
        }

        int ret = gpio_pin_configure_dt(
            buttons[i],
            GPIO_INPUT
        );

        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

int button_read_event(button_event_t *event)
{
    if (event == NULL) {
        return -EINVAL;
    }

    for (int i = 0; i < 4; i++) {

        int value = gpio_pin_get_dt(buttons[i]);

        if (value < 0) {
            return value;
        }

        if (value == 1) {

            k_msleep(BUTTON_DEBOUNCE_MS);

            value = gpio_pin_get_dt(buttons[i]);

            if (value == 1) {

                switch (i) {

                    case 0:
                        *event = EVENT_COIN_500;
                        break;

                    case 1:
                        *event = EVENT_COIN_1000;
                        break;

                    case 2:
                        *event = EVENT_COIN_2000;
                        break;

                    case 3:
                        *event = EVENT_PRODUCT;
                        break;
                }

                /* Tunggu sampai tombol dilepas */
                do {
                    k_msleep(10);
                    value = gpio_pin_get_dt(buttons[i]);
                } while (value == 1);

                return 1;
            }
        }
    }

    return 0;
}