/****************************************************************************************************
 * @name: rdk_imu_module_gpio.c
 * @author: xiaoye.zhang@Leaf from D-Robotics.
 * @version: 1.0.0
 * @date: 2026/01/19
 *
 * @description: RDK IMU Module用户态驱动载板GPIO设备部分实现
 *     开发环境为gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0
 *     依赖RDK wiringPi，gcc编译需保证系统已安装RDK wiringPi
 *     并使用-lwiringPi链接
 *
 * @note: 线程不安全，多线程需求需要自行加锁
 *
 * @note: 编译时-DDEBUG_INFO开启宏将在运行时显示DEBUG信息
 *
 ****************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>

#include "rdk_imu_module.h"

// 引脚定义
#define LED0_GPIO_PIN_NUM 17
#define LED1_GPIO_PIN_NUM 27
#define LED2_GPIO_PIN_NUM 22
#define BELL_GPIO_PIN_NUM 6
#define W1_GPIO_PIN_NUM 26

// 常量定义
#define DS18B20_PIN 26        // BCM引脚编号26，对应物理引脚37
#define ERROR_TEMP -1000.0     // 错误温度值标识
#define RESET_DELAY 480        // 复位延时（微秒）
#define PRESENCE_DELAY 70      // 存在脉冲延时（微秒）
#define CONVERSION_DELAY 750   // 温度转换时间（毫秒）

enum rdk_imu_error RDK_IMU_GPIO_Init(){
    if(wiringPiSetup()==-1)return RDK_IMU_GPIO_ERROR;

    pinMode(LED0_GPIO_PIN_NUM, OUTPUT);
    pinMode(LED1_GPIO_PIN_NUM, OUTPUT);
    pinMode(LED2_GPIO_PIN_NUM, OUTPUT);
    pinMode(BELL_GPIO_PIN_NUM, OUTPUT);
    pinMode(W1_GPIO_PIN_NUM, OUTPUT);

    digitalWrite(LED0_GPIO_PIN_NUM, HIGH);
    digitalWrite(LED1_GPIO_PIN_NUM, HIGH);
    digitalWrite(LED2_GPIO_PIN_NUM, HIGH);
    digitalWrite(BELL_GPIO_PIN_NUM, LOW);
    digitalWrite(W1_GPIO_PIN_NUM, HIGH);

    delay(100);

    return RDK_IMU_OK;
}

enum rdk_imu_error RDK_IMU_GPIO_Enable(
    enum rdk_imu_gpio_sel gpio_sel)
{
    if(gpio_sel >> 0 & 1)digitalWrite(LED0_GPIO_PIN_NUM, LOW);
    if(gpio_sel >> 1 & 1)digitalWrite(LED1_GPIO_PIN_NUM, LOW);
    if(gpio_sel >> 2 & 1)digitalWrite(LED2_GPIO_PIN_NUM, LOW);
    if(gpio_sel >> 3 & 1)digitalWrite(BELL_GPIO_PIN_NUM, HIGH);
    
    return RDK_IMU_OK;
}

enum rdk_imu_error RDK_IMU_GPIO_Disable(
    enum rdk_imu_gpio_sel gpio_sel)
{
    if(gpio_sel >> 0 & 1)digitalWrite(LED0_GPIO_PIN_NUM, HIGH);
    if(gpio_sel >> 1 & 1)digitalWrite(LED1_GPIO_PIN_NUM, HIGH);
    if(gpio_sel >> 2 & 1)digitalWrite(LED2_GPIO_PIN_NUM, HIGH);
    if(gpio_sel >> 3 & 1)digitalWrite(BELL_GPIO_PIN_NUM, LOW);
    
    return RDK_IMU_OK;
}

static int ds18b20_reset() {
    int presence = 0;

    pinMode(DS18B20_PIN, OUTPUT);   // 设置引脚为输出模式
    digitalWrite(DS18B20_PIN, LOW); // 拉低电平480微秒
    delayMicroseconds(RESET_DELAY);

    digitalWrite(DS18B20_PIN, HIGH); // 拉高电平70微秒，准备接收存在脉冲
    delayMicroseconds(PRESENCE_DELAY);

    pinMode(DS18B20_PIN, INPUT);    // 切换为输入模式

    if (digitalRead(DS18B20_PIN) == LOW) {
        presence = 1;  // 检测到低电平，存在脉冲
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Presence pulse detected.\n", __FILE__, __func__, __LINE__);
#endif
    } else {
        presence = 0;
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]No presence pulse detected.\n", __FILE__, __func__, __LINE__);
#endif
    }

    delayMicroseconds(RESET_DELAY);  // 等待结束存在脉冲时间

    return presence;
}

static void write_bit(int bit) {
    pinMode(DS18B20_PIN, OUTPUT);
    if (bit) {
        // 写入1
        digitalWrite(DS18B20_PIN, LOW);
        delayMicroseconds(1);        // 1us 拉低
        digitalWrite(DS18B20_PIN, HIGH);
        delayMicroseconds(60);       // 60us 高电平
    } else {
        // 写入0
        digitalWrite(DS18B20_PIN, LOW);
        delayMicroseconds(60);       // 60us 拉低
        digitalWrite(DS18B20_PIN, HIGH);
        delayMicroseconds(1);        // 1us 释放线
    }
}

static void write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        write_bit((byte >> i) & 0x01);
    }
}

static int read_bit() {
    int bit = 0;

    pinMode(DS18B20_PIN, OUTPUT);
    digitalWrite(DS18B20_PIN, LOW);
    delayMicroseconds(1);          // 1us 拉低

    pinMode(DS18B20_PIN, INPUT);
    delayMicroseconds(15);         // 等待15us

    bit = digitalRead(DS18B20_PIN);
    delayMicroseconds(45);         // 等待结束时间

    return bit;
}

static uint8_t read_byte() {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (read_bit() << i);
    }
    return byte;
}

enum rdk_imu_error RDK_IMU_Get_DS18B20_Temp(
    float *temp)
{
    // 复位并初始化温度转换
    if (!ds18b20_reset()){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]DS18B20 not detected. Cannot read temperature.\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_TEMP_ERROR;  // 返回一个异常值
    }

    write_byte(0xCC);  // Skip ROM
    write_byte(0x44);  // Convert T

    // 等待温度转换完成，通常750ms足够
    delay(CONVERSION_DELAY);

    // 复位并读取温度寄存器
    if (!ds18b20_reset()){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]DS18B20 not detected during temperature read.\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_TEMP_ERROR;
    }

    write_byte(0xCC);  // Skip ROM
    write_byte(0xBE);  // Read Scratchpad

    // 读取9个字节的scratchpad数据（这里只读取前两字节温度数据）
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = read_byte();
    }

    // 组合温度数据
    int16_t raw_temp = (data[1] << 8) | data[0];

    // // 处理负温度
    // if (raw_temp > 32767) {
    //     raw_temp -= 65536;
    // }

    // 转换为摄氏度（12位分辨率）
    *temp = raw_temp / 16.0;

    return RDK_IMU_OK;
}
