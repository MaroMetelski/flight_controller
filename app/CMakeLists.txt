cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/controller.c)
target_sources(app PRIVATE src/battery.c)
target_sources(app PRIVATE src/imu.c)
