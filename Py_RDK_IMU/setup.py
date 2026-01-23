#!/usr/bin/python3
# -*- coding: utf-8 -*-
# usage: sudo python3 setup.py build_ext --inplace
"""
name: setup.py

description: RDK IMU Module用户空间驱动代码Cython编译脚本

author: xiaoye.zhang@Leaf from D-Robotics.
version: 1.0.0
date: 2026/01/19
"""
from setuptools import setup, Extension
from Cython.Build import cythonize

source_files = ["Py_RDK_IMU.pyx", "../src/rdk_imu_module_data.c", "../src/rdk_imu_module_gpio.c"]

include_dirs = ["../include"]

lib_dirs = ["../lib"]

libraries = ["wiringPi", "wiringPiDev"]

ext_modules = [Extension(
    "rdkimu", 
    source_files, 
    language="c",
    include_dirs = include_dirs,
    library_dirs = lib_dirs,
    libraries = libraries)]

# import name: rdkimu
setup(name="rdkimu", ext_modules=cythonize(ext_modules))

print("\033[32mSetup has been completed!\033[0m")