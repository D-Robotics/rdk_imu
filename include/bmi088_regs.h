/****************************************************************************************************
 * @name: bmi088_regs.h
 * @author: xiaoye.zhang@Leaf from D-Robotics.
 * @version: 1.0.0
 * @date: 2026/01/14
 * @description: 定义了RDK IMU Module芯片BMI088的寄存器宏
 *       除了芯片的寄存器地址宏以外，还定义了：
 *       1.大部分寄存器的reset值
 *       2.部分寄存器具体设置的位掩码
 *       3.寄存器设置对应的枚举类型
 *
 ****************************************************************************************************/
#ifndef __BMI088_REGS_H__
#define __BMI088_REGS_H__

#include <stdint.h>

/* All mask macros only support 16-bit. */
#define BIT(_bit) ((uint16_t)1<<(_bit))
#define GENMASK(_hb, _lb) ((((uint16_t)1 << ((_hb) - (_lb) + 1)) - 1) << (_lb))

/* Accelerometer Registers */
#define BMI088_REG_ACC_SOFTRESET 0x7E
#define BMI088_REG_ACC_SOFTRESET_CMD 0xB6

#define BMI088_REG_ACC_PWR_CTRL 0x7D
#define BMI088_REG_ACC_PWR_CTRL_RESET 0x00
enum bmi088_acc_enable{
    ACC_PWR_CTRL_OFF = 0x00,
    ACC_PWR_CTRL_ON = 0x04};

#define BMI088_REG_ACC_PWR_CONF 0x7C
#define BMI088_REG_ACC_PWR_CONF_RESET 0x03
enum bmi088_acc_pwr_save{
    ACC_PWR_CONF_SUSPEND = 0x03,
    ACC_PWR_CONF_ACTIVE = 0x00};
enum bmi088_acc_pwr_mode{
    ACC_PWR_OFF,
    ACC_PWR_SUSPEND,
    ACC_PWR_ON,
};

#define BMI088_REG_ACC_SELF_TEST 0x6D
#define BMI088_REG_ACC_SELF_TEST_RESET 0x00
#define BMI088_REG_ACC_SELF_TEST_OFF 0x00
#define BMI088_REG_ACC_SELF_TEST_POSITIVE 0x0D
#define BMI088_REG_ACC_SELF_TEST_NEGATIVE 0x09

#define BMI088_REG_INT_MAP_DATA 0x58
#define BMI088_REG_INT_MAP_DATA_RESET 0x00
#define INT2_DRDY_MASK BIT(6)
#define INT1_DRD_MASK BIT(2)

#define BMI088_REG_INT2_IO_CTRL 0x54
#define BMI088_REG_INT2_IO_CTRL_RESET 0x00
#define INT2_IN_MASK BIT(4)
#define INT2_OUT_MASK BIT(3)
#define INT2_OD_MASK BIT(2)
#define INT2_OD_LVL BIT(1)

#define BMI088_REG_INT1_IO_CTRL 0x53
#define BMI088_REG_INT1_IO_CTRL_RESET 0x00
#define INT1_IN_MASK BIT(4)
#define INT1_OUT_MASK BIT(3)
#define INT1_OD_MASK BIT(2)
#define INT1_OD_LVL BIT(1)

#define BMI088_REG_ACC_RANGE 0x41
#define BMI088_REG_ACC_RANGE_RESET 0x01
#define ACC_RANGE_MASK GENMASK(1,0)
#define ACC_RANGE(_idx) ((_idx)<<0)
enum bmi088_acc_range{
    ACC_RANGE_3G,
    ACC_RANGE_6G,
    ACC_RANGE_12G,
    ACC_RANGE_24G,
};

#define BMI088_REG_ACC_CONF 0x40
#define BMI088_REG_ACC_CONF_RESET 0xA8
#define ACC_BWP_MASK GENMASK(6,4)
#define ACC_BWP(_idx) ((_idx)<<4)
enum bmi088_acc_bwp{
    ACC_BWP_OSR4,
    ACC_BWP_OSR2,
    ACC_BWP_NORMAL,
};
#define ACC_ODR_MASK GENMASK(3,0)
#define ACC_ODR(_idx) ((_idx)<<0)
enum bmi088_acc_odr{
    ACC_ODR_12_5_HZ = 0x05,
    ACC_ODR_25_HZ,
    ACC_ODR_50_HZ,
    ACC_ODR_100_HZ,
    ACC_ODR_200_HZ,
    ACC_ODR_400_HZ,
    ACC_ODR_800_HZ,
    ACC_ODR_1600_HZ,
};

#define BMI088_REG_TEMP_LSB 0x23
#define TEMPERATURE_LSB_MASK GENMASK(7,5)
#define BMI088_REG_TEMP_MSB 0x22
#define TEMPERATURE_HSB_MASK GENMASK(7,0)

#define BMI088_REG_ACC_INT_STAT_1 0x1D
#define BMI088_REG_ACC_INT_STAT_1_RESET 0x00
#define ACC_DRDY BIT(7)

#define BMI088_REG_SENSORTIME_2 0x1A
#define BMI088_REG_SENSORTIME_1 0x19
#define BMI088_REG_SENSORTIME_0 0x18

#define BMI088_REG_ACC_Z_MSB 0x17
#define BMI088_REG_ACC_Z_LSB 0x16
#define BMI088_REG_ACC_Y_MSB 0x15
#define BMI088_REG_ACC_Y_LSB 0x14
#define BMI088_REG_ACC_X_MSB 0x13
#define BMI088_REG_ACC_X_LSB 0x12

#define BMI088_REG_ACC_STATUS 0x03
#define BMI088_REG_ACC_STATUS_RESET 0x10
#define DRDY_ACC BIT(7)

#define BMI088_REG_ACC_ERR_REG 0x02
#define BMI088_REG_ACC_ERR_REG_RESET 0x00
#define ERROR_CODE_MASK GENMASK(4,2)
#define FATAL_ERR_MASK BIT(0)

#define BMI088_REG_ACC_CHIP_ID 0x00
#define BMI088_REG_ACC_CHIP_ID_RESET 0x1E
#define ACC_CHIP_ID 0x1E

/* Gyroscope Registers */
#define BMI088_REG_GYRO_SELF_TEST 0x3C

#define BMI088_REG_INT3_INT4_IO_MAP 0x18
#define BMI088_REG_INT3_INT4_IO_MAP_RESET 0x00
enum bmi088_int3_int4_io_map{
    INT3_INT4_IO_MAP_TO_ANYPIN = 0x00,
    INT3_INT4_IO_MAP_TO_INT3 = 0x01,
    INT3_INT4_IO_MAP_TO_INT4 = 0x80,
    INT3_INT4_IO_MAP_TO_INT3_INT4 = 0x81,
};

#define BMI088_REG_INT3_INT4_IO_CONF 0x16
#define BMI088_REG_INT3_INT4_IO_CONF_RESET 0x0F
#define INT4_OD_MASK BIT(3)
#define INT4_LVL_MASK BIT(2)
#define INT3_OD_MASK BIT(1)
#define INT3_LVL_MASK BIT(0)

#define BMI088_REG_GYRO_INT_CTRL 0x15
#define BMI088_REG_GYRO_INT_CTRL_RESET 0x00
enum bmi088_gyro_ready_int{
    GYRO_READY_INT_OPEN = 0x00,
    GYRO_READY_INT_CLOSR = 0x80,
};

#define BMI088_REG_GYRO_SOFTRESET 0x14
#define BMI088_REG_GYRO_SOFTRESET_CMD 0xB6

#define BMI088_REG_GYRO_LPM1 0x11
#define BMI088_REG_GYRO_LPM1_RESET 0x00
enum bmi088_gyro_lpm1{
    GYRO_LPM1_NORMAL=0x00,
    GYRO_LPM1_SUSPEND=0x80,
    GYRO_LPM1_DEEP_SUSPEND=0x20,
};

#define BMI088_REG_GYRO_BANDWIDTH 0x10
#define BMI088_REG_GYRO_BANDWIDTH_RESET 0x80
enum bmi088_gyro_bandwidth{
    GYRO_ODR_2000HZ_BANDWIDTH_532HZ,
    GYRO_ODR_2000HZ_BANDWIDTH_230HZ,
    GYRO_ODR_1000HZ_BANDWIDTH_116HZ,
    GYRO_ODR_400HZ_BANDWIDTH_47HZ,
    GYRO_ODR_200HZ_BANDWIDTH_23HZ,
    GYRO_ODR_100HZ_BANDWIDTH_12HZ,
    GYRO_ODR_200HZ_BANDWIDTH_64HZ,
    GYRO_ODR_100HZ_BANDWIDTH_32HZ,
};

#define BMI088_REG_GYRO_RANGE 0x0F
#define BMI088_REG_GYRO_RANGE_RESET 0x00
enum bmi088_gyro_range{
    GYRO_RANGE_2000DPS,
    GYRO_RANGE_1000DPS,
    GYRO_RANGE_500DPS,
    GYRO_RANGE_250DPS,
    GYRO_RANGE_125DPS,
};

#define BMI088_REG_GYRO_INT_STAT_1 0x0A
#define GYRO_DRDY_MASK BIT(7)

#define BMI088_REG_RATE_Z_MSB 0x07
#define BMI088_REG_RATE_Z_LSB 0x06
#define BMI088_REG_RATE_Y_MSB 0x05
#define BMI088_REG_RATE_Y_LSB 0x04
#define BMI088_REG_RATE_X_MSB 0x03
#define BMI088_REG_RATE_X_LSB 0x02

#define BMI088_REG_GYRO_CHIP_ID 0x00
#define BMI088_REG_GYRO_CHIP_ID_RESET 0x0F
#define GYRO_CHIP_ID 0x0F

#endif
