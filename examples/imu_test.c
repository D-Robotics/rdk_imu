/****************************************************************************************************
 * @name: bmi088_regs.h
 * @author: xiaoye.zhang@Leaf from D-Robotics.
 * @version: 1.0.0
 * @date: 2026/01/19
 *
 * @description: RDK IMU Module Cdev测试用例
 *
 ****************************************************************************************************/
#include <stdio.h>
#include <unistd.h>

#include "bmi088_regs.h"
#include "rdk_imu_module.h"

int main(void){
    enum rdk_imu_error error_code;
    
    /* 获取初始imu状态结构体 */
    struct imu_state imu_st = RDK_IMU_Get_Initial_State();

    /* 扫描所有设备 */
    error_code = RDK_IMU_All_Device_Scan(&imu_st);

    if(error_code != RDK_IMU_OK){
        printf("[%s]bmi088 device not found.\n", __FILE__);
        return -1;
    }
    else{
        printf("[%s]Successfully detected the BMI088 device.\n", __FILE__);
    }

    /* 软件复位 */
    error_code = RDK_IMU_Accel_Reset(&imu_st);
    printf("[%s]RDK_IMU_Accel_Reset %d\n", __FILE__, error_code);
    error_code = RDK_IMU_Gyro_Reset(&imu_st);
    printf("[%s]RDK_IMU_Gyro_Reset %d\n", __FILE__, error_code);

    /* 电源、量程、滤波设置 */
    error_code = RDK_IMU_Gyro_Pwr_Set(&imu_st, GYRO_LPM1_NORMAL);
    error_code |= RDK_IMU_Gyro_Config(&imu_st, GYRO_RANGE_500DPS, GYRO_ODR_1000HZ_BANDWIDTH_116HZ);
    error_code |= RDK_IMU_Accel_Pwr_Set(&imu_st, ACC_PWR_ON);
    error_code |= RDK_IMU_Accel_Config(&imu_st, ACC_RANGE_24G, ACC_BWP_OSR4, ACC_ODR_1600_HZ);
    
    if(error_code != RDK_IMU_OK){
        printf("[%s]IMU initialization failed, error code: %d\n", __FILE__, error_code);
        return -1;
    }
    printf("[%s]IMU initialization successful %d\n", __FILE__, error_code);

    /* GPIO初始化 */
    error_code = RDK_IMU_GPIO_Init();
    if(error_code != RDK_IMU_OK){
        printf("[%s]GPIO initialization failed, error code: %d\n", __FILE__, error_code);
        return -1;
    }
    printf("[%s]GPIO initialization successful %d\n", __FILE__, error_code);

    /* 声光提示 */
    for(int i=0; i<3; i++){
        RDK_IMU_GPIO_Enable(RDK_IMU_GPIO_ALL);
        usleep(100000);
        RDK_IMU_GPIO_Disable(RDK_IMU_GPIO_ALL);
        usleep(100000);
    }

    /* 环境温度获取 */
    RDK_IMU_GPIO_Enable(RDK_IMU_GPIO_BELL);
    float env_temp;
    for(int i=0; i<3; i++){
        RDK_IMU_Get_DS18B20_Temp(&env_temp);
        printf("[%s]The %dth time obtaining the ambient temperature is %f Celsius.\n", __FILE__, i, env_temp);
    }
    RDK_IMU_GPIO_Disable(RDK_IMU_GPIO_BELL);

    usleep(1000000);

    /* 获取初始imu数据包 */
    struct imu_data pkt = RDK_IMU_Get_Initial_Data();
    
    /* 循环采集样本 */
    while(1){
        printf("\033[2J\033[H"); /* 清屏 */
        RDK_IMU_Read(&imu_st, &pkt);
        printf("[imu data] accel:%7.3f, %7.3f, %7.3f, timestamp: %ld\n", pkt.accel.x, pkt.accel.y, pkt.accel.z, pkt.accel.timestamp);
        printf("[imu data]  gyro:%7.3f, %7.3f, %7.3f, timestamp: %ld\n", pkt.angvel.x, pkt.angvel.y, pkt.angvel.z, pkt.angvel.timestamp);
        printf("[temp data] temp: %f\n", pkt.temp);
        /* 间隔10ms采集 */
        usleep(10000);
    }

    return 0;
}
