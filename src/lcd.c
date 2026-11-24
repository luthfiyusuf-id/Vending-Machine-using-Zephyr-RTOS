#include "lcd.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/devicetree.h>

#define LCD_ADDR 0x27

#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BACKLIGHT 0x08

static const struct device *i2c_dev =
    DEVICE_DT_GET(DT_NODELABEL(i2c0));

static int lcd_write_byte(uint8_t data)
{
    return i2c_write(
        i2c_dev,
        &data,
        1,
        LCD_ADDR
    );
}

static int lcd_write_nibble(uint8_t data)
{
    int ret;
    data |= LCD_BACKLIGHT;
    ret = lcd_write_byte(data | LCD_EN);
    if (ret < 0) {
        return ret;
    }
    k_busy_wait(1);
    ret = lcd_write_byte(data & ~LCD_EN);
    k_busy_wait(50);
    return ret;
}

static int lcd_send(uint8_t value, uint8_t mode)
{
    int ret;

    uint8_t high = (value & 0xF0) | mode;
    uint8_t low = ((value << 4) & 0xF0) | mode;

    ret = lcd_write_nibble(high);

    if (ret < 0) {
        return ret;
    }

    return lcd_write_nibble(low);
}

static int lcd_command(uint8_t command)
{
    return lcd_send(command, 0);
}

static int lcd_data(uint8_t data)
{
    return lcd_send(data, LCD_RS);
}

int lcd_init(void)
{
    if (!device_is_ready(i2c_dev)) {
        return -ENODEV;
    }

    k_msleep(50);

    int ret;

    ret = lcd_write_nibble(0x30);
    if (ret < 0) return ret;

    k_msleep(5);

    ret = lcd_write_nibble(0x30);
    if (ret < 0) return ret;

    k_busy_wait(150);

    ret = lcd_write_nibble(0x30);
    if (ret < 0) return ret;

    ret = lcd_write_nibble(0x20);
    if (ret < 0) return ret;

    ret = lcd_command(0x28);
    if (ret < 0) return ret;

    ret = lcd_command(0x0C);
    if (ret < 0) return ret;

    ret = lcd_command(0x06);
    if (ret < 0) return ret;

    return lcd_clear();
}

int lcd_clear(void)
{
    int ret = lcd_command(0x01);

    k_msleep(2);

    return ret;
}

int lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    if (row > 1 || col > 15) {
        return -EINVAL;
    }

    address = (row == 0) ? col : (0x40 + col);

    return lcd_command(0x80 | address);
}

int lcd_print(const char *text)
{
    if (text == NULL) {
        return -EINVAL;
    }

    while (*text) {

        int ret = lcd_data((uint8_t)*text);

        if (ret < 0) {
            return ret;
        }

        text++;
    }

    return 0;
}

int lcd_print_at(uint8_t row, uint8_t col,
                 const char *text)
{
    int ret = lcd_set_cursor(row, col);

    if (ret < 0) {
        return ret;
    }

    return lcd_print(text);
}
void lcd_clear_line(uint8_t row)
{
    lcd_set_cursor(row, 0);
    lcd_print("                "); // 16 spasi
}