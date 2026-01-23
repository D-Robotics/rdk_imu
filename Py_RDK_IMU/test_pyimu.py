#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
name: setup.py

description: RDK IMU Module用户空间驱动代码Python测试用例

author: xiaoye.zhang@Leaf from D-Robotics.
version: 1.0.0
date: 2026/01/19
"""
import rdkimu
import time

def blink():
    imu_gpio.GPIO_Ctrl(
        led_r_st = rdkimu.GPIO_State._open,
        led_b_st = rdkimu.GPIO_State._close,
        led_g_st = rdkimu.GPIO_State._close,
        bell_st = rdkimu.GPIO_State._open)
    time.sleep(0.1)
    imu_gpio.GPIO_Ctrl(
        led_r_st = rdkimu.GPIO_State._close,
        led_b_st = rdkimu.GPIO_State._close,
        led_g_st = rdkimu.GPIO_State._close,
        bell_st = rdkimu.GPIO_State._close)
    time.sleep(0.1)

    imu_gpio.GPIO_Ctrl(
        led_r_st = rdkimu.GPIO_State._close,
        led_b_st = rdkimu.GPIO_State._open,
        led_g_st = rdkimu.GPIO_State._close,
        bell_st = rdkimu.GPIO_State._open)
    time.sleep(0.1)
    imu_gpio.GPIO_Ctrl(
        led_r_st = rdkimu.GPIO_State._close,
        led_b_st = rdkimu.GPIO_State._close,
        led_g_st = rdkimu.GPIO_State._close,
        bell_st = rdkimu.GPIO_State._close)
    time.sleep(0.1)

    imu_gpio.GPIO_Ctrl(
        led_r_st = rdkimu.GPIO_State._close,
        led_b_st = rdkimu.GPIO_State._close,
        led_g_st = rdkimu.GPIO_State._open,
        bell_st = rdkimu.GPIO_State._open)
    time.sleep(0.1)
    imu_gpio.GPIO_Ctrl(
        led_r_st = rdkimu.GPIO_State._close,
        led_b_st = rdkimu.GPIO_State._close,
        led_g_st = rdkimu.GPIO_State._close,
        bell_st = rdkimu.GPIO_State._close)
    time.sleep(0.1)

if __name__ == "__main__":
    # 初始化imu类
    imu = rdkimu.RDK_IMU()
    # 初始化imu gpio类
    imu_gpio = rdkimu.RDK_IMU_GPIO()

    # imu地址扫描
    ret, info = imu.Device_Scan()
    if ret:
        print("imu地址扫描成功, 地址信息如下：")
        print(info)
    else:
        print("未扫描到imu设备")
        exit()

    # 参数配置
    imu.Accel_Reset()
    imu.Gyro_Reset()
    ret = imu.Set_Pwr_Mode(
        rdkimu.Accel_Pwr.pwr_on, rdkimu.Gyro_Pwr.pwr_on) and \
        imu.Accel_Config() and \
        imu.Gyro_Config(odr_bwp = rdkimu.Gyro_Odr_Bwp._1000hz_116hz)

    if ret:
        print("imu初始化成功, 配置如下：")
        print(imu.Get_IMU_Info())
    else:
        print("imu初始化失败")
        exit()

    # 声光提示
    blink()

    # 检测板载温度
    print("板载温度：", imu_gpio.Get_Board_Temp())
    time.sleep(1)

    while True:
        ret = imu.Data_Update()
        if not ret:
            print("IMU数据更新失败")
            exit()

        imu_data = imu.Data_Read()
        for key in imu_data:
            print(key, imu_data[key])

        time.sleep(0.01)
