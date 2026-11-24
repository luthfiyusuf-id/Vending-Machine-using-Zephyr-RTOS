cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(vending_machine)

target_sources(app PRIVATE
    src/main.c
    src/state_machine.c
    src/button.c
    src/lcd.c
    src/servo.c
)