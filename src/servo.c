#include "servo.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/devicetree.h>

#define SERVO_PERIOD_NS 20000000U

#define SERVO_MIN_PULSE_US 500
#define SERVO_MAX_PULSE_US 2400

static const struct pwm_dt_spec servo =
    PWM_DT_SPEC_GET(DT_ALIAS(servo_pwm));

int servo_init(void)
{
    if (!pwm_is_ready_dt(&servo)) {
        return -ENODEV;
    }

    return servo_set_angle(0);
}

int servo_set_angle(int angle)
{
    if (angle < 0) {
        angle = 0;
    }

    if (angle > 180) {
        angle = 180;
    }

    uint32_t pulse_us =
        SERVO_MIN_PULSE_US +
        ((SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)
        * angle / 180);

    uint32_t pulse_ns = pulse_us * 1000U;

    return pwm_set_dt(
        &servo,
        SERVO_PERIOD_NS,
        pulse_ns
    );
}

int servo_dispense(void)
{
    int ret;

    /* Posisi awal */
    ret = servo_set_angle(0);

    if (ret < 0) {
        return ret;
    }

    k_msleep(500);

    /* Membuka mekanisme */
    ret = servo_set_angle(90);

    if (ret < 0) {
        return ret;
    }

    k_msleep(1500);

    /* Menutup mekanisme */
    ret = servo_set_angle(0);

    if (ret < 0) {
        return ret;
    }

    k_msleep(500);

    return 0;
}