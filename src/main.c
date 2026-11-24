#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "button.h"
#include "lcd.h"
#include "servo.h"
#include "state_machine.h"

#define INPUT_STACK_SIZE 2048
#define INPUT_PRIORITY 4

K_THREAD_STACK_DEFINE(
    input_stack,
    INPUT_STACK_SIZE
);

static struct k_thread input_thread_data;

/* Thread pembacaan tombol */
static void input_thread(void *arg1,
                         void *arg2,
                         void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    button_event_t event;

    while (1) {

        int ret = button_read_event(&event);

        if (ret == 1) {

            int queue_ret =
                state_machine_post_event(event);

            if (queue_ret < 0) {
                printk("Event queue penuh!\n");
            }
        }

        else if (ret < 0) {

            printk(
                "Error membaca tombol: %d\n",
                ret
            );

            k_msleep(100);
        }

        k_msleep(20);
    }
}

int main(void)
{
    int ret;

    printk("\n");
    printk("============================\n");
    printk("  VENDING MACHINE ZEPHYR\n");
    printk("============================\n");

    /* Inisialisasi tombol */
    ret = button_init();

    if (ret < 0) {

        printk("Button initialization failed\n");

        return ret;
    }

    printk("Button initialized\n");

    /* Inisialisasi LCD */
    ret = lcd_init();

    if (ret < 0) {

        printk("LCD initialization failed\n");

        return ret;
    }

    printk("LCD initialized\n");

    /* Inisialisasi servo */
    ret = servo_init();

    if (ret < 0) {

        printk("Servo initialization failed\n");

        return ret;
    }

    printk("Servo initialized\n");

    /* Membuat input thread */
    k_thread_create(
        &input_thread_data,
        input_stack,
        K_THREAD_STACK_SIZEOF(input_stack),
        input_thread,
        NULL,
        NULL,
        NULL,
        INPUT_PRIORITY,
        0,
        K_NO_WAIT
    );

    printk("Input thread started\n");

    /* Main thread menjalankan FSM */
    state_machine_run();

    return 0;
}