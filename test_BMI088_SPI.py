#!/usr/bin/env python3

import sys
import signal
import time
import Hobot.GPIO as GPIO
import spidev

# Signal handler for safe exit
def signal_handler(signal, frame):
    sys.exit(0)

class BMI088:
    def __init__(self, acc_cs_pin, gyro_cs_pin, spi_bus=0, spi_device=0, spi_speed=5000000):
        """
        Initialize BMI088 class and setup the SPI communication along with GPIO pins.
        :param acc_cs_pin: Chip select pin for the accelerometer
        :param gyro_cs_pin: Chip select pin for the gyroscope
        :param spi_bus: SPI bus number, default is 0
        :param spi_device: SPI device number, default is 0
        :param spi_speed: SPI communication speed, default is 5 MHz
        """
        self.acc_cs_pin = acc_cs_pin
        self.gyro_cs_pin = gyro_cs_pin
        
        # Initialize GPIO pins
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(self.acc_cs_pin, GPIO.OUT, initial=GPIO.LOW)
        GPIO.setup(self.gyro_cs_pin, GPIO.OUT, initial=GPIO.LOW)
        
        # Initialize SPI communication
        self.spi = spidev.SpiDev()
        self.spi.open(spi_bus, spi_device)
        self.spi.max_speed_hz = spi_speed
        self.spi.mode = 0b00  # SPI 模式 0
        
        # BMI088 register addresses and device IDs
        self.ACC_CHIP_ID = 0x00
        self.GYRO_CHIP_ID = 0x00
        self.EXPECTED_ACC_ID = 0x1E
        self.EXPECTED_GYRO_ID = 0x0F
        self.ACC_X_LSB = 0x12
        self.ACC_Y_LSB = 0x14  # Accelerometer Y-axis register address
        self.ACC_Z_LSB = 0x16  # Accelerometer Z-axis register address
        self.GYRO_X_LSB = 0x02
        self.GYRO_Y_LSB = 0x04  # Gyroscope Y-axis register address
        self.GYRO_Z_LSB = 0x06  # Gyroscope Z-axis register address

    def read_register(self, register, cs_pin):
        """Read data from a specified register."""
        GPIO.output(cs_pin, GPIO.LOW)
        resp = self.spi.xfer2([register | 0x80, 0x00])
        GPIO.output(cs_pin, GPIO.HIGH)
        return resp[1]

    def write_register(self, register, value, cs_pin):
        """Write data to a specified register."""
        GPIO.output(cs_pin, GPIO.LOW)
        self.spi.xfer2([register & 0x7F, value])
        GPIO.output(cs_pin, GPIO.HIGH)
    
    def initialize(self):
        """Initialize the accelerometer and gyroscope."""
        # Initialize accelerometer
        self.write_register(0x7D, 0x04, self.acc_cs_pin) # 0x04 normal mode
        time.sleep(0.01)
        
        # Initialize gyroscope
        self.write_register(0x15, 0x04, self.gyro_cs_pin) 
        time.sleep(0.01)

    def read_chip_ids(self):
        """Read and verify the chip IDs of the accelerometer and gyroscope."""
        self.write_register(0x7D, 0x00, self.acc_cs_pin) 
        self.write_register(0x15, 0x00, self.gyro_cs_pin) 
        
        acc_chip_id = self.read_register(self.ACC_CHIP_ID, self.acc_cs_pin)
        gyro_chip_id = self.read_register(self.GYRO_CHIP_ID, self.gyro_cs_pin)
        
        print(f"Accelerometer ID: {hex(acc_chip_id)} (Expected: {hex(self.EXPECTED_ACC_ID)})")
        print(f"Gyroscope ID: {hex(gyro_chip_id)} (Expected: {hex(self.EXPECTED_GYRO_ID)})")
        
        if acc_chip_id == self.EXPECTED_ACC_ID and gyro_chip_id == self.EXPECTED_GYRO_ID:
            print("BMI088 initialized successfully.")
        else:
            print("BMI088 initialization failed. Check connections or configuration.")

    def read_accel(self):
        """Read and return the X, Y, Z axis data from the accelerometer."""
        acc_x = self._combine_register_values(self.ACC_X_LSB, self.acc_cs_pin)
        acc_y = self._combine_register_values(self.ACC_Y_LSB, self.acc_cs_pin)
        acc_z = self._combine_register_values(self.ACC_Z_LSB, self.acc_cs_pin)
        return acc_x, acc_y, acc_z

    def read_gyro(self):
        """Read and return the X, Y, Z axis data from the gyroscope."""
        gyro_x = self._combine_register_values(self.GYRO_X_LSB, self.gyro_cs_pin)
        gyro_y = self._combine_register_values(self.GYRO_Y_LSB, self.gyro_cs_pin)
        gyro_z = self._combine_register_values(self.GYRO_Z_LSB, self.gyro_cs_pin)
        return gyro_x, gyro_y, gyro_z
    
    def _combine_register_values(self, reg, cs_pin):
        """Helper function to read LSB and MSB, and combine them into a signed 16-bit value."""
        lsb = self.read_register(reg, cs_pin)
        msb = self.read_register(reg + 1, cs_pin)
        value = (msb << 8) | lsb
        if value > 32767:
            value -= 65536
        return value

    def close(self):
        """Close the SPI connection and clean up the GPIO pins."""
        self.spi.close()
        GPIO.cleanup()
    
    
def main():
    acc_cs_pin = 24
    gyro_cs_pin = 26
    spi_bus = 1
    spi_device = 0
    
    try:
        # Create BMI088 object, passing the CS pins for the accelerometer and gyroscope
        bmi088 = BMI088(acc_cs_pin=acc_cs_pin, gyro_cs_pin=gyro_cs_pin, spi_bus=spi_bus, spi_device=spi_device)
        
        # Initialize sensors and read IDs
        bmi088.read_chip_ids()
        
        bmi088.initialize()
        
        # Continuously read accelerometer and gyroscope data
        while True:
            acc_x, acc_y, acc_z = bmi088.read_accel()
            gyro_x, gyro_y, gyro_z = bmi088.read_gyro()
            print(f"Accelerometer - X: {acc_x}, Y: {acc_y}, Z: {acc_z}")
            print(f"Gyroscope - X: {gyro_x}, Y: {gyro_y}, Z: {gyro_z}")
            time.sleep(0.1)  # Read data every 100ms

    except KeyboardInterrupt:
        # Close sensors
        bmi088.close()

    
    
if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    main()