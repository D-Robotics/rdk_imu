#include <wiringPi.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define OUTPUT_PIN1 12  // GPIO 17 (BCM引脚号, 对应 BOARD 编号 11)
#define OUTPUT_PIN2 13  // GPIO 27 (BCM引脚号, 对应 BOARD 编号 13)
#define OUTPUT_PIN3 30  // GPIO 22 (BCM引脚号, 对应 BOARD 编号 15)
#define OUTPUT_PIN4 118 // GPIO 6  (BCM引脚号, 对应 BOARD 编号 31)


int main() {
    // 初始化 WiringPi 并设置为 BCM 编码模式
    if (wiringPiSetupGpio() == -1) {
        printf("Failed to initialize WiringPi.\n");
        return -1;
    }

    // 设置输出引脚并初始化为低电平
    pinMode(OUTPUT_PIN1, OUTPUT);
    pinMode(OUTPUT_PIN2, OUTPUT);
    pinMode(OUTPUT_PIN3, OUTPUT);
    pinMode(OUTPUT_PIN4, OUTPUT);

    digitalWrite(OUTPUT_PIN1, LOW);
    digitalWrite(OUTPUT_PIN2, LOW);
    digitalWrite(OUTPUT_PIN3, LOW);
    digitalWrite(OUTPUT_PIN4, LOW);

    printf("GPIO Pins initialized and set to LOW.\n");
    return 0;
}
