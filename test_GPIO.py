#!/usr/bin/env python3
import sys
import signal
import Hobot.GPIO as GPIO

def signal_handler(signal, frame):
    sys.exit(0)

# 定义使用的GPIO通道为38
output_pin1 = 11 # BOARD 编码 11
output_pin2 = 13 # BOARD 编码 13
output_pin3 = 15 # BOARD 编码 15
output_pin4 = 31 # BOARD 编码 31


def main():
    # 设置管脚编码模式为硬件编号 BOARD
    GPIO.setmode(GPIO.BOARD)
    # 设置为输出模式，并且初始化为高电平
    GPIO.setup(output_pin1, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(output_pin2, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(output_pin3, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(output_pin4, GPIO.OUT, initial=GPIO.LOW)
    GPIO.output(output_pin1, GPIO.LOW)
    GPIO.output(output_pin2, GPIO.LOW)
    GPIO.output(output_pin3, GPIO.LOW)
    GPIO.output(output_pin4, GPIO.LOW)
    
    GPIO.cleanup()

if __name__=='__main__':
    signal.signal(signal.SIGINT, signal_handler)
    main()