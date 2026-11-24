#include "state_machine.h"
#include "lcd.h"
#include "servo.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <stdio.h>
#include <string.h>

#define PRODUCT_PRICE 5000

#define MESSAGE_QUEUE_SIZE 10

K_MSGQ_DEFINE(
    event_queue,
    sizeof(button_event_t),
    MESSAGE_QUEUE_SIZE,
    4
);

static vending_state_t current_state = STATE_IDLE;

static int balance = 0;

static int change = 0;

/* Mengirim event ke message queue */
int state_machine_post_event(button_event_t event)
{
    return k_msgq_put(
        &event_queue,
        &event,
        K_NO_WAIT
    );
}

/* Menampilkan dua baris LCD */
static void lcd_show(const char *line1,
                     const char *line2)
{
    char buffer[17];

    memset(buffer, ' ', 16);
    buffer[16] = '\0';

    strncpy(buffer, line1, 16);

    lcd_print_at(0, 0, buffer);

    memset(buffer, ' ', 16);
    buffer[16] = '\0';

    strncpy(buffer, line2, 16);

    lcd_print_at(1, 0, buffer);
}

/* Menampilkan harga dan saldo */
static void display_balance(void)
{
    char line1[17];
    char line2[17];

    snprintf(
        line1,
        sizeof(line1),
        "Harga: Rp%d    ",
        PRODUCT_PRICE
    );

    snprintf(
        line2,
        sizeof(line2),
        "Saldo: Rp%d    ",
        balance
    );

    lcd_show(line1, line2);
}

/* Menghitung nominal uang */
static int get_coin_value(button_event_t event)
{
    switch (event) {

        case EVENT_COIN_500:
            return 500;

        case EVENT_COIN_1000:
            return 1000;

        case EVENT_COIN_2000:
            return 2000;

        default:
            return 0;
    }
}

/* Memproses input pengguna */
static void process_event(button_event_t event)
{
    int coin = get_coin_value(event);

    /* STATE IDLE dan CREDIT */
    if (current_state == STATE_IDLE ||
        current_state == STATE_CREDIT) {

        if (coin > 0) {

            balance += coin;

            current_state = STATE_CREDIT;

            printk("Saldo: Rp%d    \n", balance);

            display_balance();

            return;
        }
    }

    /* Pemilihan produk */
    if (event == EVENT_PRODUCT) {

        if (balance >= PRODUCT_PRICE) {

            current_state = STATE_DISPENSE;

        } else {

            lcd_show(

                "Saldo kurang!    ",
                "Masukkan koin   "
            );

            k_msleep(1200);

            current_state =
                (balance > 0) ?
                STATE_CREDIT : STATE_IDLE;

            display_balance();
        }
    }
}

/* Menjalankan proses pengeluaran barang */
static void process_dispense(void)
{
    printk("Memproses produk...\n");

    lcd_show(

        "Memproses...",
        "Tunggu sebentar"
    );

    int ret = servo_dispense();

    if (ret < 0) {

        lcd_show(

            "Servo error!      ",
            "Transaksi batal  "
        );

        printk("Servo error: %d\n", ret);

        balance = 0;
        change = 0;

        current_state = STATE_IDLE;

        k_msleep(2000);

        return;
    }

    current_state = STATE_COMPLETE;
}

/* Menyelesaikan transaksi */
static void process_complete(void)
{
    change = balance - PRODUCT_PRICE;

    char line2[17];

    snprintf(
        line2,
        sizeof(line2),
        "Kembali:%d     ",
        change
    );

    lcd_show(
        "Transaksi OK!    ",
        line2
    );

    printk("Transaksi selesai\n");
    printk("Harga: Rp%d    \n", PRODUCT_PRICE);
    printk("Saldo: Rp%d    \n", balance);
    printk("Kembalian: Rp%d    \n", change);

    k_msleep(2500);

    /* Reset transaksi */
    balance = 0;
    change = 0;

    current_state = STATE_IDLE;

    /* Buang event lama saat transaksi berlangsung */
    k_msgq_purge(&event_queue);

    lcd_show(
      
        "VENDING MACHINE",
        "Masukkan koin"
    );
}

/* Menjalankan FSM */
void state_machine_run(void)
{
    button_event_t event;

    lcd_show(

        "VENDING MACHINE",
        "Masukkan koin"
    );

    while (1) {

        switch (current_state) {

            case STATE_IDLE:

                if (k_msgq_get(
                    &event_queue,
                    &event,
                    K_MSEC(100)
                ) == 0) {

                    process_event(event);
                }

                break;

            case STATE_CREDIT:

                if (k_msgq_get(
                    &event_queue,
                    &event,
                    K_MSEC(100)
                ) == 0) {

                    process_event(event);
                }

                break;

            case STATE_DISPENSE:

                process_dispense();

                break;

            case STATE_COMPLETE:

                process_complete();

                break;

            default:

                balance = 0;

                current_state = STATE_IDLE;

                break;
        }
    }
}