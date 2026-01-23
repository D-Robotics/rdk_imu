/****************************************************************************************************
 * @name: rdk_imu_module.h
 * @author: xiaoye.zhang@Leaf from D-Robotics.
 * @version: 1.0.0
 * @date: 2026/01/19
 *
 * @description: 定义了RDK IMU Module所有外设功能函数
 *       主要功能包括：
 *       1.IMU数据读出与解析
 *       2.RDK IMU Module载板GPIO外设驱动
 *       需注意：       
 *       - 由于用户态权限限制，未实现中断功能
 *       - 以下几个函数依赖RDK wiringPi:
 *           * enum rdk_imu_error RDK_IMU_GPIO_Init();
 *
 *           * enum rdk_imu_error RDK_IMU_GPIO_Enable(
 *                enum rdk_imu_gpio_sel gpio_sel);
 *
 *           * enum rdk_imu_error RDK_IMU_GPIO_Disable(
 *               enum rdk_imu_gpio_sel gpio_sel);
 *
 *           * enum rdk_imu_error RDK_IMU_Get_DS18B20_Temp(
 *               float *temp);
 *       - 如需使用则需要：
 *           1.编译附带源文件rdk_imu_module_gpio.c 或
 *           2.静态链接rdk_imu_module_gpio.a 或
 *           3.动态链接rdk_imu_module_gpio.so
 *       - 并链接wiringPi
 *
 * @note: 线程不安全，多线程需求需要自行加锁
 *
 ****************************************************************************************************/
#ifndef __RDK_IMU_MODULE_H__
#define __RDK_IMU_MODULE_H__

#include <stdio.h>
#include <stdbool.h>

#include "bmi088_regs.h"

/* 单位：us */
#define IMU_ACCEL_RESET_DELAY 1000
#define IMU_ACCEL_PWR_DELAY 50000
#define IMU_GYRO_RESET_DELAY 30000
#define IMU_GYRO_PWR_DELAY 30000

/***************************************************
 * @enum imu_transmit_interface
 *
 * @brief IMU使用的通信方式
 *
 ***************************************************/
enum imu_transmit_interface{
    IMU_TSMT_INTF_SPI,
    IMU_TSMT_INTF_I2C,
};

/***************************************************
 * @enum rdk_imu_error
 *
 * @brief rdk imu库错误类型枚举
 *
 ***************************************************/
enum rdk_imu_error{
    RDK_IMU_OK = 0,
    RDK_IMU_BUS_FAULT, // 总线错误/通信错误
    RDK_IMU_HARD_FAULT, // 硬件错误
    RDK_IMU_INVALID_PARAM, // 无效的参数
    RDK_IMU_TIMEOUT, // 超时错误
    RDK_IMU_NO_SUPPORT, // 不支持的操作
    RDK_IMU_NOT_INITED, // 缺少有效初始化
    RDK_IMU_ACCEL_ERROR, // 加速度计配置错误
    RDK_IMU_GPIO_ERROR, // GPIO错误
    RDK_IMU_TEMP_ERROR,
};

/***************************************************
 * @enum rdk_imu_gpio_sel
 *
 * @brief rdk imu模组载板GPIO外设选择
 *
 ***************************************************/
enum rdk_imu_gpio_sel{
    RDK_IMU_GPIO_NULL = 0x00,

    RDK_IMU_GPIO_LED0 = 0x01,
    RDK_IMU_GPIO_LED1 = 0x02,
    RDK_IMU_GPIO_LED2 = 0x04,
    RDK_IMU_GPIO_BELL = 0x08,

    RDK_IMU_GPIO_01 = 0x03,
    RDK_IMU_GPIO_02 = 0x05,
    RDK_IMU_GPIO_0B = 0x09,
    RDK_IMU_GPIO_12 = 0x06,
    RDK_IMU_GPIO_1B = 0x0A,
    RDK_IMU_GPIO_2B = 0x0C,

    RDK_IMU_GPIO_012 = 0x07,
    RDK_IMU_GPIO_01B = 0x0B,
    RDK_IMU_GPIO_02B = 0x0D,
    RDK_IMU_GPIO_12B = 0x0E,

    RDK_IMU_GPIO_ALL = 0x0F,
};

/***************************************************
 * @struct imu_state
 * @brief IMU状态结构体
 * 
 * @details 包含IMU模块的完整状态（不含中断）
 * 
 * @member: 通信相关
 *  tsmt_intf: @enum imu_transmit_interface 通信接口类型
 *
 *  spi_accel_fd: spi加速度计片选的字符设备文件变量
 *  spi_gyro_fd: spi陀螺仪片选的字符设备文件变量
 *  spi_clock_speed: spi时钟频率
 *
 *  i2c_accel_bus: i2c加速度计所在bus号
 *  i2c_gyro_bus: i2c陀螺仪所在bus号
 *  i2c_accel_addr: 加速度计i2c地址
 *  i2c_gyro_addr: 陀螺仪i2c地址
 *  i2c_accel_fd: i2c加速度计的字符设备文件变量
 *  i2c_gyro_fd: i2c陀螺仪的字符设备文件变量
 *
 * @member: imu片上状态相关
 *  acc_pwr_mode: accel电源模式
 *      @enum bmi088_acc_pwr_mode 
 *  gyro_pwr_mode: gyro低功耗模式
 *      @enum bmi088_gyro_lpm1 
 *
 *  acc_range: accel量程
 *      @enum bmi088_acc_range 
 *  acc_bwp: accel滤波器设置
 *      @enum bmi088_acc_bwp 
 *  acc_odr: accel采样率
 *      @enum bmi088_acc_odr 
 *
 *  gyro_range: gyro量程
 *      @enum bmi088_gyro_range 
 *  gyro_bandwidth: gyro采样率与低通滤波带宽设置
 *      @enum bmi088_gyro_bandwidth 
 *
 * @member: 时域处理算法相关
 *  sensor_time_scale_numerator: sensor单位时间缩放系数 分子(us)
 *      @note 使用约分计算器将浮点缩放系数转换为本参数
 *  sensor_time_scale_denominator: sensor单位时间缩放系数 分母
 *      @note 使用约分计算器将浮点缩放系数转换为本参数
 *
 *  time_sync_weight: 时间同步权重
 *      @note 数值越大，越相信IMU的ODR时间数据 
 *
 *  read_timeout: 数据包继承timeout(us)
 *      @note 使用函数@RDK_IMU_Read时读取数据时，当检测到两帧时间差超
 *          过该值时，不继承数据
 *
 ***************************************************/
struct imu_state{
    enum imu_transmit_interface tsmt_intf;

    int spi_accel_fd;
    int spi_gyro_fd;
    uint32_t spi_clock_speed;

    uint8_t i2c_accel_bus;
    uint8_t i2c_gyro_bus;
    uint8_t i2c_accel_addr;
    uint8_t i2c_gyro_addr;
    int i2c_accel_fd;
    int i2c_gyro_fd;

    enum bmi088_acc_pwr_mode acc_pwr_mode;
    enum bmi088_gyro_lpm1 gyro_pwr_mode;

    enum bmi088_acc_range acc_range;
    enum bmi088_acc_bwp acc_bwp;
    enum bmi088_acc_odr acc_odr;

    enum bmi088_gyro_range gyro_range;
    enum bmi088_gyro_bandwidth gyro_bandwidth;

    uint16_t sensor_time_scale_numerator;
    uint16_t sensor_time_scale_denominator;

    float time_sync_weight;

    uint32_t read_timeout;
};

/***************************************************
 * @struct imu_3_axis_data
 * @brief 3轴数据包结构体
 * 
 * @details 包含一个三轴独立数据包的完整数据
 * 
 * @member: 
 *  x/y/z: 三轴浮点数值，单位不在此处定义
 *  timestamp: 实时时间戳数据
 *
 ***************************************************/
struct imu_3_axis_data{
    float x, y, z;
    uint64_t timestamp;
};

/***************************************************
 * @struct imu_data
 * @brief imu数据结构体
 * 
 * @details 包含imu数据包的完整数据
 * 
 * @member: 
 *  accel/angvel: 加速度/角速度数据包
 *      @struct imu_3_axis_data
 *
 *  temp: 实时结温
 *
 *  sys_timestamp: 系统时间戳记录
 *      @note 将由函数@RDK_IMU_Read记录数据读取前一刻的系统时间戳
 *
 *  imu_sensortime: 片上时间戳记录
 *      @note 片上24位加速度计原始时间戳
 *      @note 与三轴加速度数据强相关
 *      @note 由于是原始数据，每经过片上655.36s将会溢出
 *
 ***************************************************/
struct imu_data{
    struct imu_3_axis_data accel;
    struct imu_3_axis_data angvel;
    float temp;
    uint64_t sys_timestamp;
    uint32_t imu_sensortime;
};

/***************************************************
 * @name RDK_IMU_Get_Initial_State
 * @brief IMU状态结构体初始化函数
 * 
 * @details 初始化@struct:imu_state的标准方式
 *
 * @return 经正确初始化，默认状态的@struct:imu_state
 *
 ***************************************************/
struct imu_state RDK_IMU_Get_Initial_State();

/***************************************************
 * @name RDK_IMU_Get_Initial_Data
 * @brief IMU数据结构体初始化函数
 * 
 * @details 获取@struct:imu_data初始包的标准方式
 *
 * @return 经正确初始化，默认状态的@struct:imu_data
 *
 ***************************************************/
struct imu_data RDK_IMU_Get_Initial_Data();

/***************************************************
 * @name RDK_IMU_All_Device_Scan
 * @brief 全设备扫描函数
 * 
 * @details 自动扫描总线上连接的imu设备并检查chip id
 *      支持i2c/spi自动识别
 *      支持i2c全总线全地址自动扫描
 *      spi仅支持spidev1.0和spidev1.1
 *      支持spi双片选号自动判断并交换
 *
 * @param st: imu_state结构体指针
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_All_Device_Scan(
    struct imu_state *st);

/***************************************************
 * @name RDK_IMU_Accel_Pwr_Set
 * @brief 加速度计电源模式设置
 * 
 * @param st: imu_state结构体指针
 * @param pwr_mode: @enum bmi088_acc_pwr_mode accel电源模式
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Accel_Pwr_Set(
    struct imu_state *st,
    enum bmi088_acc_pwr_mode pwr_mode);

/***************************************************
 * @name RDK_IMU_Gyro_Pwr_Set
 * @brief 陀螺仪电源模式设置
 * 
 * @param st: imu_state结构体指针
 * @param pwr_mode: @enum bmi088_gyro_lpm1 gyro电源模式
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Gyro_Pwr_Set(
    struct imu_state *st,
    enum bmi088_gyro_lpm1 pwr_mode);

/***************************************************
 * @name RDK_IMU_Accel_Config
 * @brief 加速度计配置
 * 
 * @param st: imu_state结构体指针
 * @param range: @enum bmi088_acc_range accel量程
 * @param bwp: @enum bmi088_acc_bwp accel滤波设置
 * @param odr: @enum bmi088_acc_odr accel采样频率
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Accel_Config(
    struct imu_state *st,
    enum bmi088_acc_range range,
    enum bmi088_acc_bwp bwp,
    enum bmi088_acc_odr odr);

/***************************************************
 * @name RDK_IMU_Gyro_Config
 * @brief 陀螺仪配置
 * 
 * @param st: imu_state结构体指针
 * @param range: @enum bmi088_gyro_range gyro量程
 * @param bandwidth @enum bmi088_gyro_bandwidth gyro采用频率与带宽
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Gyro_Config(
    struct imu_state *st,    
    enum bmi088_gyro_range range,
    enum bmi088_gyro_bandwidth bandwidth);

/***************************************************
 * @name RDK_IMU_Accel_Reset
 * @brief 加速度计软件复位
 * 
 * @param st: imu_state结构体指针
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Accel_Reset(
    struct imu_state *st);

/***************************************************
 * @name RDK_IMU_Gyro_Reset
 * @brief 陀螺仪软件复位
 * 
 * @param st: imu_state结构体指针
 *
 * @return rdk_imu_error 错误码
 *
 * @todo gyro在i2c模式下无法成功软复位，原因未知
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Gyro_Reset(
    struct imu_state *st);

/***************************************************
 * @name RDK_IMU_Read
 * @brief IMU完整数据采集函数
 * 
 * @param st: imu_state结构体指针
 * @param data: imu_data数据包结构体指针
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Read(
    struct imu_state *st,
    struct imu_data *data);

/***************************************************
 * @name RDK_IMU_GPIO_Init
 * @brief IMU载板GPIO初始化
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_GPIO_Init();

/***************************************************
 * @name RDK_IMU_GPIO_Enable
 * @brief IMU载板GPIO使能
 *
 * @details 已处理好逻辑0-1转换
 *
 * @param gpio_sel 要选择的gpio掩码
 *     @enum rdk_imu_gpio_sel
 *
 * @return rdk_imu_error 错误码
 *
 * @usage: 使能led0和蜂鸣器，不影响其他gpio
 *  enum rdk_imu_error ret = RDK_IMU_GPIO_Enable(
 *      RDK_IMU_GPIO_LED0 | RDK_IMU_GPIO_BELL);
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_GPIO_Enable(
    enum rdk_imu_gpio_sel gpio_sel);

/***************************************************
 * @name RDK_IMU_GPIO_Disable
 * @brief IMU载板GPIO关闭
 *
 * @details 已处理好逻辑0-1转换
 *
 * @param gpio_sel 要选择的gpio掩码
 *     @enum rdk_imu_gpio_sel
 *
 * @return rdk_imu_error 错误码
 *
 * @usage: 关闭led0和蜂鸣器，不影响其他gpio
 *  enum rdk_imu_error ret = RDK_IMU_GPIO_Disable(
 *      RDK_IMU_GPIO_LED0 | RDK_IMU_GPIO_BELL);
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_GPIO_Disable(
    enum rdk_imu_gpio_sel gpio_sel);

/***************************************************
 * @name RDK_IMU_Get_DS18B20_Temp
 * @brief IMU载板温度传感器数据读取
 *
 * @note 软件1-wire总线，耗时较长，错误率高
 *
 * @param temp 浮点温度数据变量指针
 *
 * @return rdk_imu_error 错误码
 *
 ***************************************************/
enum rdk_imu_error RDK_IMU_Get_DS18B20_Temp(
    float *temp);

#endif
