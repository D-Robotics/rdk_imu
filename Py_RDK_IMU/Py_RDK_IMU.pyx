# distutils: language = c
# cython: language_level=3
# -*- coding: utf-8 -*-
"""
name: Py_RDK_IMU.pyx

description: RDK IMU Module用户空间驱动代码Cython接口封装

author: xiaoye.zhang@Leaf from D-Robotics.
version: 1.0.0
date: 2026/01/19
"""
from libc.stdint cimport uint8_t, uint16_t, uint32_t, uint64_t 
from enum import Enum, unique

cdef extern from "bmi088_regs.h":
    cdef enum bmi088_acc_pwr_mode:
        ACC_PWR_OFF
        ACC_PWR_SUSPEND
        ACC_PWR_ON

    cdef enum bmi088_acc_range:
        ACC_RANGE_3G
        ACC_RANGE_6G
        ACC_RANGE_12G
        ACC_RANGE_24G

    cdef enum bmi088_acc_bwp:
        ACC_BWP_OSR4
        ACC_BWP_OSR2
        ACC_BWP_NORMAL

    cdef enum bmi088_acc_odr:
        ACC_ODR_12_5_HZ = 0x05
        ACC_ODR_25_HZ
        ACC_ODR_50_HZ
        ACC_ODR_100_HZ
        ACC_ODR_200_HZ
        ACC_ODR_400_HZ
        ACC_ODR_800_HZ
        ACC_ODR_1600_HZ

    cdef enum bmi088_gyro_lpm1:
        GYRO_LPM1_NORMAL = 0x00
        GYRO_LPM1_SUSPEND = 0x80
        GYRO_LPM1_DEEP_SUSPEND = 0x20

    cdef enum bmi088_gyro_bandwidth:
        GYRO_ODR_2000HZ_BANDWIDTH_532HZ
        GYRO_ODR_2000HZ_BANDWIDTH_230HZ
        GYRO_ODR_1000HZ_BANDWIDTH_116HZ
        GYRO_ODR_400HZ_BANDWIDTH_47HZ
        GYRO_ODR_200HZ_BANDWIDTH_23HZ
        GYRO_ODR_100HZ_BANDWIDTH_12HZ
        GYRO_ODR_200HZ_BANDWIDTH_64HZ
        GYRO_ODR_100HZ_BANDWIDTH_32HZ

    cdef enum bmi088_gyro_range:
        GYRO_RANGE_2000DPS
        GYRO_RANGE_1000DPS
        GYRO_RANGE_500DPS
        GYRO_RANGE_250DPS
        GYRO_RANGE_125DPS

cdef extern from "rdk_imu_module.h":
    cdef enum rdk_imu_error:
        RDK_IMU_OK = 0
        RDK_IMU_BUS_FAULT
        RDK_IMU_HARD_FAULT
        RDK_IMU_INVALID_PARAM
        RDK_IMU_TIMEOUT
        RDK_IMU_NO_SUPPORT
        RDK_IMU_NOT_INITED
        RDK_IMU_ACCEL_ERROR
        RDK_IMU_GPIO_ERROR
        RDK_IMU_TEMP_ERROR

    cdef enum imu_transmit_interface:
        IMU_TSMT_INTF_SPI
        IMU_TSMT_INTF_I2C

    cdef struct imu_state:
        imu_transmit_interface tsmt_intf
        int spi_accel_fd
        int spi_gyro_fd
        uint32_t spi_clock_speed
        uint8_t i2c_accel_bus
        uint8_t i2c_gyro_bus
        uint8_t i2c_accel_addr
        uint8_t i2c_gyro_addr
        int i2c_accel_fd
        int i2c_gyro_fd
        bmi088_acc_pwr_mode acc_pwr_mode
        bmi088_gyro_lpm1 gyro_pwr_mode
        bmi088_acc_range acc_range
        bmi088_acc_bwp acc_bwp
        bmi088_acc_odr acc_odr
        bmi088_gyro_range gyro_range
        bmi088_gyro_bandwidth gyro_bandwidth
        uint16_t sensor_time_scale_numerator
        uint16_t sensor_time_scale_denominator
        float time_sync_weight
        uint32_t read_timeout

    cdef struct imu_3_axis_data:
        float x, y, z
        uint64_t timestamp

    cdef struct imu_data:
        imu_3_axis_data accel
        imu_3_axis_data angvel
        float temp
        uint64_t sys_timestamp
        uint32_t imu_sensortime

    cdef imu_state RDK_IMU_Get_Initial_State()

    cdef imu_data RDK_IMU_Get_Initial_Data()

    cdef rdk_imu_error RDK_IMU_All_Device_Scan(
        imu_state *)

    cdef rdk_imu_error RDK_IMU_Accel_Pwr_Set(
        imu_state *,
        bmi088_acc_pwr_mode)

    cdef rdk_imu_error RDK_IMU_Gyro_Pwr_Set(
        imu_state *,
        bmi088_gyro_lpm1)

    cdef rdk_imu_error RDK_IMU_Accel_Config(
        imu_state *,
        bmi088_acc_range,
        bmi088_acc_bwp,
        bmi088_acc_odr)

    cdef rdk_imu_error RDK_IMU_Gyro_Config(
        imu_state *,    
        bmi088_gyro_range,
        bmi088_gyro_bandwidth)

    cdef rdk_imu_error RDK_IMU_Accel_Reset(
        imu_state *)

    cdef rdk_imu_error RDK_IMU_Gyro_Reset(
        imu_state *)

    cdef rdk_imu_error RDK_IMU_Read(
        imu_state *,
        imu_data *)

    cdef rdk_imu_error RDK_IMU_GPIO_Init()

    cdef enum rdk_imu_gpio_sel:
        RDK_IMU_GPIO_NULL = 0x00

        RDK_IMU_GPIO_LED0 = 0x01
        RDK_IMU_GPIO_LED1 = 0x02
        RDK_IMU_GPIO_LED2 = 0x04
        RDK_IMU_GPIO_BELL = 0x08

        RDK_IMU_GPIO_01 = 0x03
        RDK_IMU_GPIO_02 = 0x05
        RDK_IMU_GPIO_0B = 0x09
        RDK_IMU_GPIO_12 = 0x06
        RDK_IMU_GPIO_1B = 0x0A
        RDK_IMU_GPIO_2B = 0x0C

        RDK_IMU_GPIO_012 = 0x07
        RDK_IMU_GPIO_01B = 0x0B
        RDK_IMU_GPIO_02B = 0x0D
        RDK_IMU_GPIO_12B = 0x0E

        RDK_IMU_GPIO_ALL = 0x0F

    cdef rdk_imu_error RDK_IMU_GPIO_Enable(
        rdk_imu_gpio_sel)

    cdef rdk_imu_error RDK_IMU_GPIO_Disable(
        rdk_imu_gpio_sel)

    cdef rdk_imu_error RDK_IMU_Get_DS18B20_Temp(
        float *)

@unique
class IMU_Interface(Enum):
    spi = "spi"
    i2c = "i2c"
    unknow = "unknow"

@unique
class Accel_Range(Enum):
    _3g = 3.0
    _6g = 6.0
    _12g = 12.0
    _24g = 24.0

@unique
class Gyro_Range(Enum):
    _2000dps = 2000.0
    _1000dps = 1000.0
    _500dps = 500.0
    _250dps = 250.0
    _125dps = 125.0

@unique
class Accel_Bwp(Enum):
    osr4 = "osr4"
    osr2 = "osr2"
    normal = "normal"

@unique
class Accel_Odr(Enum):
    _12_5hz = 12.5
    _25hz = 25.0
    _50hz = 50.0
    _100hz = 100.0
    _200hz = 200.0
    _400hz = 400.0
    _800hz = 800.0
    _1600hz = 1600.0

@unique
class Gyro_Odr_Bwp(Enum):
    _2000hz_532hz = (2000.0, 532.0)
    _2000hz_230hz = (2000.0, 230.0)
    _1000hz_116hz = (1000.0, 116.0)
    _400hz_47hz = (400.0, 47.0)
    _200hz_23hz = (200.0, 23.0)
    _100hz_12hz = (100.0, 12.0)
    _200hz_64hz = (200.0, 64.0)
    _100hz_32hz = (100.0, 32.0)

@unique
class Accel_Pwr(Enum):
    pwr_on = "on"
    pwr_suspend = "suspend"
    pwr_off = "off"

@unique
class Gyro_Pwr(Enum):
    pwr_on = "on"
    pwr_suspend = "suspend"
    pwr_deepsuspend = "deepsuspend"

@unique
class GPIO_State(Enum):
    _open = "open"
    _close = "close"
    _hold_on = "hold on"

cdef class RDK_IMU:
    cdef imu_state imu_st
    cdef imu_data imu_dt

    def __cinit__(self):
        self.imu_st = RDK_IMU_Get_Initial_State()
        self.imu_dt = RDK_IMU_Get_Initial_Data()

    def Device_Scan(self) -> Tuple[bool, Optional[Dict[str, Any]]]:
        cdef rdk_imu_error cret = RDK_IMU_All_Device_Scan(&self.imu_st)
        if cret != RDK_IMU_OK:
            return False, None

        info = {}
        if self.imu_st.tsmt_intf == IMU_TSMT_INTF_SPI:
            info["Interface"] = IMU_Interface.spi
            info["Accel_Path"] = "/dev/spidev1.x"
            info["Gyro_Path"] = "/dev/spidev1.x"
        elif self.imu_st.tsmt_intf == IMU_TSMT_INTF_I2C:
            info["Interface"] = IMU_Interface.i2c
            info["AccelBusNumber"] = int(self.imu_st.i2c_accel_bus)
            info["GyroBusNumber"] = int(self.imu_st.i2c_gyro_bus)
            info["AccelAddr"] = int(self.imu_st.i2c_accel_addr)
            info["GyroAddr"] = int(self.imu_st.i2c_gyro_addr)
        else:
            raise TypeError("A communication structure error occurred while scanning the device.")

        return True, info

    def Accel_Reset(self) -> bool:
        if RDK_IMU_Accel_Reset(&self.imu_st) == RDK_IMU_OK:
            return True
        else:
            return False

    def Gyro_Reset(self) -> bool:
        if RDK_IMU_Gyro_Reset(&self.imu_st) == RDK_IMU_OK:
            return True
        else:
            return False

    def Set_Pwr_Mode(self, 
        accel_pwr:Accel_Pwr, 
        gyro_pwr:Gyro_Pwr) -> bool:

        if isinstance(accel_pwr, Accel_Pwr) != True or isinstance(accel_pwr, Accel_Pwr) != True:
            raise TypeError("Function parameter type error.")
            
        cdef rdk_imu_error cret = <rdk_imu_error>0

        if accel_pwr == Accel_Pwr.pwr_on:
            cret = RDK_IMU_Accel_Pwr_Set(&self.imu_st, ACC_PWR_ON);
        elif accel_pwr == Accel_Pwr.pwr_suspend:
            cret = RDK_IMU_Accel_Pwr_Set(&self.imu_st, ACC_PWR_SUSPEND);
        elif accel_pwr == Accel_Pwr.pwr_off:
            cret = RDK_IMU_Accel_Pwr_Set(&self.imu_st, ACC_PWR_OFF);

        if cret != RDK_IMU_OK:
            return False
        
        if gyro_pwr == Gyro_Pwr.pwr_on:
            cret = RDK_IMU_Gyro_Pwr_Set(&self.imu_st, GYRO_LPM1_NORMAL)
        elif gyro_pwr == Gyro_Pwr.pwr_suspend:
            cret = RDK_IMU_Gyro_Pwr_Set(&self.imu_st, GYRO_LPM1_SUSPEND)
        elif gyro_pwr == Gyro_Pwr.pwr_deepsuspend:
            cret = RDK_IMU_Gyro_Pwr_Set(&self.imu_st, GYRO_LPM1_DEEP_SUSPEND)

        if cret != RDK_IMU_OK:
            return False
        else:
            return True

    def Accel_Config(self, 
        range:Accel_Range = Accel_Range._24g,
        bwp:Accel_Bwp = Accel_Bwp.normal,
        odr:Accel_Odr = Accel_Odr._1600hz) -> bool:

        if isinstance(range, Accel_Range) != True or isinstance(bwp, Accel_Bwp) != True or isinstance(odr, Accel_Odr) != True:
            raise TypeError("Function parameter type error.")
            
        cdef rdk_imu_error cret = <rdk_imu_error>0

        cdef bmi088_acc_range crange = <bmi088_acc_range>0
        cdef bmi088_acc_bwp cbwp = <bmi088_acc_bwp>0
        cdef bmi088_acc_odr codr = <bmi088_acc_odr>0

        if range == Accel_Range._3g:
            crange = ACC_RANGE_3G
        elif range == Accel_Range._6g:
            crange = ACC_RANGE_6G
        elif range == Accel_Range._12g:
            crange = ACC_RANGE_12G
        elif range == Accel_Range._24g:
            crange = ACC_RANGE_24G

        if bwp == Accel_Bwp.osr4:
            cbwp = ACC_BWP_OSR4
        elif bwp == Accel_Bwp.osr2:
            cbwp = ACC_BWP_OSR2
        elif bwp == Accel_Bwp.normal:
            cbwp = ACC_BWP_NORMAL

        if odr == Accel_Odr._12_5hz:
            codr = ACC_ODR_12_5_HZ
        if odr == Accel_Odr._25hz:
            codr = ACC_ODR_25_HZ
        if odr == Accel_Odr._50hz:
            codr = ACC_ODR_50_HZ
        if odr == Accel_Odr._100hz:
            codr = ACC_ODR_100_HZ
        if odr == Accel_Odr._200hz:
            codr = ACC_ODR_200_HZ
        if odr == Accel_Odr._400hz:
            codr = ACC_ODR_400_HZ
        if odr == Accel_Odr._800hz:
            codr = ACC_ODR_800_HZ
        if odr == Accel_Odr._1600hz:
            codr = ACC_ODR_1600_HZ

        cret = RDK_IMU_Accel_Config(&self.imu_st, crange, cbwp, codr);

        if cret != RDK_IMU_OK:
            return False
        else:
            return True
        
    def Gyro_Config(self, 
        range:Gyro_Range = Gyro_Range._2000dps,
        odr_bwp:Gyro_Odr_Bwp = Gyro_Odr_Bwp._2000hz_532hz) -> bool:

        if(isinstance(range, Gyro_Range) != True or isinstance(odr_bwp, Gyro_Odr_Bwp)) != True:
            raise TypeError("Function parameter type error.")

        cdef rdk_imu_error cret = <rdk_imu_error>0

        cdef bmi088_gyro_range crange = <bmi088_gyro_range>0
        cdef bmi088_gyro_bandwidth codr_bwp = <bmi088_gyro_bandwidth>0

        if range == Gyro_Range._2000dps:
            crange = GYRO_RANGE_2000DPS
        elif range == Gyro_Range._1000dps:
            crange = GYRO_RANGE_1000DPS
        elif range == Gyro_Range._500dps:
            crange = GYRO_RANGE_500DPS
        elif range == Gyro_Range._250dps:
            crange = GYRO_RANGE_250DPS
        elif range == Gyro_Range._125dps:
            crange = GYRO_RANGE_125DPS

        if odr_bwp == Gyro_Odr_Bwp._2000hz_532hz:
            codr_bwp = GYRO_ODR_2000HZ_BANDWIDTH_532HZ
        if odr_bwp == Gyro_Odr_Bwp._2000hz_230hz:
            codr_bwp = GYRO_ODR_2000HZ_BANDWIDTH_230HZ
        if odr_bwp == Gyro_Odr_Bwp._1000hz_116hz:
            codr_bwp = GYRO_ODR_1000HZ_BANDWIDTH_116HZ
        if odr_bwp == Gyro_Odr_Bwp._400hz_47hz:
            codr_bwp = GYRO_ODR_400HZ_BANDWIDTH_47HZ
        if odr_bwp == Gyro_Odr_Bwp._200hz_23hz:
            codr_bwp = GYRO_ODR_200HZ_BANDWIDTH_23HZ
        if odr_bwp == Gyro_Odr_Bwp._100hz_12hz:
            codr_bwp = GYRO_ODR_100HZ_BANDWIDTH_12HZ
        if odr_bwp == Gyro_Odr_Bwp._200hz_64hz:
            codr_bwp = GYRO_ODR_200HZ_BANDWIDTH_64HZ
        if odr_bwp == Gyro_Odr_Bwp._100hz_32hz:
            codr_bwp = GYRO_ODR_100HZ_BANDWIDTH_32HZ

        cret = RDK_IMU_Gyro_Config(&self.imu_st, crange, codr_bwp);

        if cret != RDK_IMU_OK:
            return False
        else:
            return True
            
    def Get_IMU_Info(self) -> Dict[str, Any]:
        info = {}

        if self.imu_st.acc_pwr_mode == ACC_PWR_OFF:
            info["AccPwrMode"] = Accel_Pwr.pwr_off
        elif self.imu_st.acc_pwr_mode == ACC_PWR_SUSPEND:
            info["AccPwrMode"] = Accel_Pwr.pwr_suspend
        elif self.imu_st.acc_pwr_mode == ACC_PWR_ON:
            info["AccPwrMode"] = Accel_Pwr.pwr_on

        if self.imu_st.acc_range == ACC_RANGE_3G:
            info["AccRange"] = Accel_Range._3g
        elif self.imu_st.acc_range == ACC_RANGE_6G:
            info["AccRange"] = Accel_Range._6g
        elif self.imu_st.acc_range == ACC_RANGE_12G:
            info["AccRange"] = Accel_Range._12g
        elif self.imu_st.acc_range == ACC_RANGE_24G:
            info["AccRange"] = Accel_Range._24g

        if self.imu_st.acc_bwp == ACC_BWP_OSR4:
            info["AccBwp"] = Accel_Bwp.osr4
        elif self.imu_st.acc_bwp == ACC_BWP_OSR2:
            info["AccBwp"] = Accel_Bwp.osr2
        elif self.imu_st.acc_bwp == ACC_BWP_NORMAL:
            info["AccBwp"] = Accel_Bwp.normal

        if self.imu_st.gyro_pwr_mode == GYRO_LPM1_NORMAL:
            info["GyroPwr"] = Gyro_Pwr.pwr_on
        elif self.imu_st.gyro_pwr_mode == GYRO_LPM1_SUSPEND:
            info["GyroPwr"] = Gyro_Pwr.pwr_suspend
        elif self.imu_st.gyro_pwr_mode == GYRO_LPM1_DEEP_SUSPEND:
            info["GyroPwr"] = Gyro_Pwr.pwr_deepsuspend

        if self.imu_st.gyro_range == GYRO_RANGE_2000DPS:
            info["GyroRange"] = Gyro_Range._2000dps
        elif self.imu_st.gyro_range == GYRO_RANGE_1000DPS:
            info["GyroRange"] = Gyro_Range._1000dps
        elif self.imu_st.gyro_range == GYRO_RANGE_500DPS:
            info["GyroRange"] = Gyro_Range._500dps
        elif self.imu_st.gyro_range == GYRO_RANGE_250DPS:
            info["GyroRange"] = Gyro_Range._250dps
        elif self.imu_st.gyro_range == GYRO_RANGE_125DPS:
            info["GyroRange"] = Gyro_Range._125dps

        if self.imu_st.gyro_bandwidth == GYRO_ODR_2000HZ_BANDWIDTH_532HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._2000hz_532hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_2000HZ_BANDWIDTH_230HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._2000hz_230hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_1000HZ_BANDWIDTH_116HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._1000hz_116hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_400HZ_BANDWIDTH_47HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._400hz_47hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_200HZ_BANDWIDTH_23HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._200hz_23hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_100HZ_BANDWIDTH_12HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._100hz_12hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_200HZ_BANDWIDTH_64HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._200hz_64hz
        elif self.imu_st.gyro_bandwidth == GYRO_ODR_100HZ_BANDWIDTH_32HZ:
            info["GyroOdrBandwidth"] = Gyro_Odr_Bwp._100hz_32hz

        return info
    
    def Get_TimeSync_Weight(self) -> float:
        return float(self.imu_st.time_sync_weight)

    def Set_TimeSync_Weight(self,
        w:float) -> None:
        if isinstance(w, float):
            raise TypeError("Function parameter type error.")
        self.imu_st.time_sync_weight = w;

    def Get_Read_Timeout(self) -> int:
        return int(self.imu_st.read_timeout)

    def Set_Read_Timeout(self,
        timeout:uint32_t) -> None:
        self.imu_st.read_timeout = timeout

    def Get_SPI_Speed(self) -> int:
        return int(self.imu_st.spi_clock_speed)

    def Set_SPI_Speed(self,
        speed:uint32_t) -> None:
        self.imu_st.spi_clock_speed = speed

    def Data_Update(self) -> Tuple[bool, int]:
        cdef rdk_imu_error cret
        cret = RDK_IMU_Read(&self.imu_st, &self.imu_dt)
        if cret == RDK_IMU_OK:
            return True, 0
        else:
            return False, int(cret)

    def Data_Read(self) -> Dict[str, Any]:
        return {
            "Accel":{
                "x":float(self.imu_dt.accel.x),
                "y":float(self.imu_dt.accel.y),
                "z":float(self.imu_dt.accel.z),
                "timestamp":int(self.imu_dt.accel.timestamp),
            },
            "AngVel":{
                "x":float(self.imu_dt.angvel.x),
                "y":float(self.imu_dt.angvel.y),
                "z":float(self.imu_dt.angvel.z),
                "timestamp":int(self.imu_dt.angvel.timestamp),
            },
            "Temp":float(self.imu_dt.temp),
            "UpdateTimestamp":int(self.imu_dt.sys_timestamp),
        }

cdef class RDK_IMU_GPIO:
    def __cinit__(self):
        cdef rdk_imu_error cret = RDK_IMU_GPIO_Init();
        if cret != RDK_IMU_OK:
            raise RuntimeError("An error occurred during GPIO initialization.");

    def Get_Board_Temp(self) -> Tuple[bool, float]:
        cdef float temp;
        cdef rdk_imu_error cret = RDK_IMU_Get_DS18B20_Temp(&temp);

        if cret != RDK_IMU_OK or temp == -1000.0:
            return False, 0.0
        else:
            return True, float(temp)

    def GPIO_Ctrl(self,
        led_r_st:GPIO_State = GPIO_State._hold_on,
        led_b_st:GPIO_State = GPIO_State._hold_on,
        led_g_st:GPIO_State = GPIO_State._hold_on,
        bell_st:GPIO_State = GPIO_State._hold_on) -> bool:
        
        cdef rdk_imu_gpio_sel close_mask = RDK_IMU_GPIO_NULL
        if led_r_st == GPIO_State._close:
            close_mask = <rdk_imu_gpio_sel>(close_mask | RDK_IMU_GPIO_LED0)
        if led_b_st == GPIO_State._close:
            close_mask = <rdk_imu_gpio_sel>(close_mask | RDK_IMU_GPIO_LED1)
        if led_g_st == GPIO_State._close:
            close_mask = <rdk_imu_gpio_sel>(close_mask | RDK_IMU_GPIO_LED2)
        if bell_st == GPIO_State._close:
            close_mask = <rdk_imu_gpio_sel>(close_mask | RDK_IMU_GPIO_BELL)

        cdef rdk_imu_gpio_sel open_mask = RDK_IMU_GPIO_NULL
        if led_r_st == GPIO_State._open:
            open_mask = <rdk_imu_gpio_sel>(open_mask | RDK_IMU_GPIO_LED0)
        if led_b_st == GPIO_State._open:
            open_mask = <rdk_imu_gpio_sel>(open_mask | RDK_IMU_GPIO_LED1)
        if led_g_st == GPIO_State._open:
            open_mask = <rdk_imu_gpio_sel>(open_mask | RDK_IMU_GPIO_LED2)
        if bell_st == GPIO_State._open:
            open_mask = <rdk_imu_gpio_sel>(open_mask | RDK_IMU_GPIO_BELL)

        cdef rdk_imu_error cret;
        
        cret = RDK_IMU_GPIO_Disable(close_mask);

        if cret != RDK_IMU_OK:
            return False

        cret = RDK_IMU_GPIO_Enable(open_mask);

        if cret != RDK_IMU_OK:
            return False

        return True
