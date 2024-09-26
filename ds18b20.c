#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>  // for sleep and usleep

// 常量定义
#define DS18B20_PIN 117        // BCM引脚编号26，对应物理引脚37
#define ERROR_TEMP -1000.0     // 错误温度值标识
#define RESET_DELAY 480        // 复位延时（微秒）
#define PRESENCE_DELAY 70      // 存在脉冲延时（微秒）
#define CONVERSION_DELAY 750   // 温度转换时间（毫秒）

// 初始化DS18B20传感器
void ds18b20_init() {
    pinMode(DS18B20_PIN, OUTPUT);
    digitalWrite(DS18B20_PIN, HIGH);  // 设置初始状态为高电平
    delay(100);  // 等待传感器稳定
}

// 复位DS18B20并检测存在脉冲
int ds18b20_reset() {
    int presence = 0;

    pinMode(DS18B20_PIN, OUTPUT);   // 设置引脚为输出模式
    digitalWrite(DS18B20_PIN, LOW); // 拉低电平480微秒
    delayMicroseconds(RESET_DELAY);

    digitalWrite(DS18B20_PIN, HIGH); // 拉高电平70微秒，准备接收存在脉冲
    delayMicroseconds(PRESENCE_DELAY);

    pinMode(DS18B20_PIN, INPUT);    // 切换为输入模式

    if (digitalRead(DS18B20_PIN) == LOW) {
        presence = 1;  // 检测到低电平，存在脉冲
        printf("Presence pulse detected.\n");
    } else {
        presence = 0;
        printf("No presence pulse detected.\n");
    }

    delayMicroseconds(RESET_DELAY);  // 等待结束存在脉冲时间

    return presence;
}

// 写入一个位
void write_bit(int bit) {
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

// 写入一个字节，LSB优先
void write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        write_bit((byte >> i) & 0x01);
    }
}

// 读取一个位
int read_bit() {
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

// 读取一个字节，LSB优先
uint8_t read_byte() {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (read_bit() << i);
    }
    return byte;
}

// 读取温度
float read_temperature() {
    // 复位并初始化温度转换
    if (!ds18b20_reset()) {
        printf("DS18B20 not detected. Cannot read temperature.\n");
        return ERROR_TEMP;  // 返回一个异常值
    }

    write_byte(0xCC);  // Skip ROM
    write_byte(0x44);  // Convert T

    // 等待温度转换完成，通常750ms足够
    delay(CONVERSION_DELAY);

    // 复位并读取温度寄存器
    if (!ds18b20_reset()) {
        printf("DS18B20 not detected during temperature read.\n");
        return ERROR_TEMP;
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

    // 处理负温度
    if (raw_temp > 32767) {
        raw_temp -= 65536;
    }

    // 转换为摄氏度（12位分辨率）
    float temperature = raw_temp / 16.0;

    return temperature;
}

int main() {
    // 初始化 WiringPi 库
    if (wiringPiSetupGpio() == -1) {  // 使用 BCM 引脚编号
        printf("Failed to initialize WiringPi.\n");
        return -1;
    }

    // 初始化DS18B20
    ds18b20_init();

    // 循环读取温度数据
    while (1) {
        float temperature = read_temperature();
        if (temperature != ERROR_TEMP) {  // 检查是否读取成功
            printf("Temperature: %.4f °C\n", temperature);
        } else {
            printf("Failed to read temperature.\n");
        }
        delay(1000);  // 每秒读取一次温度数据
    }

    return 0;
}
