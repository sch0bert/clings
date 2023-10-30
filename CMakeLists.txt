#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(clings_app_core)

target_sources(app PRIVATE 
    src/main.c
    src/lis2dtw12.c
    src/lis2dtw12_i2c.c
    )
target_include_directories(app PRIVATE include)