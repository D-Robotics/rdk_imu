/****************************************************************************************************
 * @name: rdk_imu_module_data.c
 * @author: xiaoye.zhang@Leaf from D-Robotics.
 * @version: 1.0.0
 * @date: 2026/01/19
 *
 * @description: RDK IMU Module用户态驱动IMU数据部分实现
 *     开发环境为gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0
 *     仅依赖spi/i2c的linux通用驱动
 *     gcc编译需要-lm链接math库
 *
 * @note: 线程不安全，多线程需求需要自行加锁
 *
 * @note: 编译时-DDEBUG_INFO开启宏将在运行时显示DEBUG信息
 *
 ****************************************************************************************************/
#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "rdk_imu_module.h"
#include "bmi088_regs.h"

static int SPI_Transfer(
    int fd,
    uint32_t spi_speed,
    const uint8_t *tx, 
    uint8_t *rx, 
    uint8_t len)
{
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
        tr.tx_buf = (unsigned long)tx;
        tr.rx_buf = (unsigned long)rx;
        tr.len = len;
        tr.delay_usecs = 0;
        tr.word_delay_usecs = 10;
        tr.speed_hz = spi_speed;
        tr.bits_per_word = 8;
        tr.cs_change = 0;

    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

static int I2C_Read_Register(
    int i2c_fd, 
    uint8_t device_addr, 
    uint8_t reg_addr, 
    uint8_t *buffer, 
    uint8_t len)
{
    struct i2c_msg messages[2];
    struct i2c_rdwr_ioctl_data packet;
    
    // 第一条消息：写入寄存器地址
    messages[0].addr = device_addr;   // 设备地址
    messages[0].flags = 0;            // 写操作
    messages[0].len = 1;              // 寄存器地址长度
    messages[0].buf = &reg_addr;      // 寄存器地址
    
    // 第二条消息：读取数据
    messages[1].addr = device_addr;   // 设备地址
    messages[1].flags = I2C_M_RD;     // 读操作
    messages[1].len = len;            // 要读取的字节数
    messages[1].buf = buffer;         // 数据缓冲区
    
    // 设置 I2C_RDWR 数据结构
    packet.msgs = messages;
    packet.nmsgs = 2;
    
    // 执行组合的读写操作
    if (ioctl(i2c_fd, I2C_RDWR, &packet) < 0) {
        // perror("I2C_RDWR failed");
        return -1;
    }
    
    return 0;
}

static int I2C_Write_Register(
    int i2c_fd, 
    uint8_t device_addr,                  
    uint8_t reg_addr, 
    const uint8_t *data, 
    uint8_t len) 
{
    struct i2c_msg message;
    struct i2c_rdwr_ioctl_data packet;
    uint8_t buffer[len + 1];
    
    // 构造数据缓冲区：寄存器地址 + 数据
    buffer[0] = reg_addr;
    for (int i = 0; i < len; i++) {
        buffer[i + 1] = data[i];
    }
    
    // 设置消息
    message.addr = device_addr;
    message.flags = 0;  // 写操作
    message.len = len + 1;
    message.buf = buffer;
    
    // 设置 I2C_RDWR 数据结构
    packet.msgs = &message;
    packet.nmsgs = 1;
    
    // 执行写操作
    if (ioctl(i2c_fd, I2C_RDWR, &packet) < 0) {
        // perror("I2C_RDWR write failed");
        return -1;
    }
    
    return 0;
}

struct imu_state RDK_IMU_Get_Initial_State()
{
    struct imu_state st;

    st.tsmt_intf = IMU_TSMT_INTF_SPI;

    st.spi_accel_fd = 0;
    st.spi_gyro_fd = 0;
    st.spi_clock_speed = 1e7;

    st.i2c_accel_addr = 0x00;
    st.i2c_gyro_addr = 0x00;
    st.i2c_accel_fd = 0;
    st.i2c_gyro_fd = 0;
    st.i2c_accel_bus = -1;
    st.i2c_gyro_bus = -1;

    st.acc_pwr_mode = ACC_PWR_OFF;
    st.gyro_pwr_mode = GYRO_LPM1_NORMAL;

    st.acc_range = ACC_RANGE_6G;
    st.acc_bwp = ACC_BWP_NORMAL;
    st.acc_odr = ACC_ODR_100_HZ;

    st.gyro_range = GYRO_RANGE_2000DPS;
    st.gyro_bandwidth = GYRO_ODR_2000HZ_BANDWIDTH_532HZ;

    st.sensor_time_scale_numerator = 625;
    st.sensor_time_scale_denominator = 16;

    st.time_sync_weight = 0.1f;

    st.read_timeout = 1e5;

    return st;
}

struct imu_data RDK_IMU_Get_Initial_Data()
{
    struct imu_data dt;
    
    dt.temp = -275.0f;
    dt.sys_timestamp = 0;
    dt.imu_sensortime = 0;
    dt.accel.x = 0.0f;
    dt.accel.y = 0.0f;
    dt.accel.z = 0.0f;
    dt.accel.timestamp = 0;
    dt.angvel.x = 0.0f;
    dt.angvel.y = 0.0f;
    dt.angvel.z = 0.0f;
    dt.angvel.timestamp = 0;

    return dt;
}

enum rdk_imu_error RDK_IMU_All_Device_Scan(
    struct imu_state *st)
{
    int ret;
    int fd0, fd1;
    uint8_t spi_mode = SPI_MODE_0;
    uint8_t spi_bit = 8;
    uint8_t rx[2], tx[2];
    char i2c_device_name[16];

    usleep(IMU_ACCEL_RESET_DELAY);
    usleep(IMU_GYRO_RESET_DELAY);

    // i2c_find: 
    /* 确保i2c无效 */
    st->i2c_accel_fd = 0;
    st->i2c_gyro_fd = 0;
    st->i2c_accel_addr = 0;
    st->i2c_gyro_addr = 0;
    /* 扫描0-8号总线 */
    for(int i=0; i<=8; i++){
        /* 正确构建设备文件名 */
        snprintf(i2c_device_name, sizeof(i2c_device_name), "/dev/i2c-%d", i);
        /* 打开设备文件 */
        fd0 = open(i2c_device_name, O_RDWR);
        if(fd0 < 0){
#ifdef DEBUG_INFO
            printf("[%s@%s:%d]Failed to open device file: /dev/i2c-%d.\n", __FILE__, __func__, __LINE__, i);
#endif
            continue;
        }
        else{
#ifdef DEBUG_INFO
            printf("[%s@%s:%d]Successfully opened device file: /dev/i2c-%d.\n", __FILE__, __func__, __LINE__, i);
#endif
        }
        /* 地址扫描，使用芯片对应的CHIP_ID寄存器校对 */
        for(uint8_t addr = 0; addr < 0x80; addr++){
            ret = I2C_Read_Register(fd0, addr, BMI088_REG_ACC_CHIP_ID, &rx[0], 1);
            ret |= I2C_Read_Register(fd0, addr, BMI088_REG_GYRO_CHIP_ID, &rx[1], 1);
            if(ret < 0)continue;
            if(rx[0] == ACC_CHIP_ID){ /* 扫描到正确的accel chip id */
#ifdef DEBUG_INFO
                printf("[%s@%s:%d]Accel Device successfully scanned, in i2c-%d bus, device addr: %#x\n", __FILE__, __func__, __LINE__, i, addr);
#endif
                st->i2c_accel_fd = fd0;
                st->i2c_accel_addr = addr;
                st->i2c_accel_bus = i;
            }
            if(rx[1] == GYRO_CHIP_ID){ /* 扫描到正确的gyro chip id */
#ifdef DEBUG_INFO
                printf("[%s@%s:%d]Gyro Device successfully scanned, in i2c-%d bus, device addr: %#x\n", __FILE__, __func__, __LINE__, i, addr);
#endif
                st->i2c_gyro_fd = fd0;
                st->i2c_gyro_addr = addr;
                st->i2c_gyro_bus = i;
            }

            /* 当accel和gyro都已被扫描发现 */
            if(st->i2c_accel_fd != 0 && st->i2c_gyro_fd != 0 &&
                st->i2c_accel_addr != 0 && st->i2c_gyro_addr != 0){
                /* 同一总线校验 */
                if(st->i2c_accel_bus != st->i2c_gyro_bus){
                    return RDK_IMU_BUS_FAULT;
                }
                /* 执行退出操作 */
                st->tsmt_intf = IMU_TSMT_INTF_I2C;
                return RDK_IMU_OK;
            }
        }
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]IMU was not found on i2c-%d bus.\n", __FILE__, __func__, __LINE__, i);
#endif
        close(fd0);
    }
    /* 程序执行到此处说明没有找到有效的I2C设备 */
    /* 开始SPI查找 */
    // spi_find:
    /* 尝试打开SPI设备 */
    /* 当前仅支持"/dev/spidev1.0"和"/dev/spidev1.1" */
    /* 确保spi无效 */
    st->spi_gyro_fd = 0;
    st->spi_accel_fd = 0;
    fd0 = open("/dev/spidev1.0",O_RDWR);
    fd1 = open("/dev/spidev1.1",O_RDWR);

    if(fd0 < 0 || fd1 < 0){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Failed to open SPI device file.\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_BUS_FAULT;
    }

    ret = 0;
    /* 设置spi传输模式 */
    ret |= ioctl(fd0, SPI_IOC_WR_MODE, &spi_mode);
    ret |= ioctl(fd1, SPI_IOC_WR_MODE, &spi_mode);
    /* 设置spi字位数 */
    ret |= ioctl(fd0, SPI_IOC_WR_BITS_PER_WORD, &spi_bit);
    ret |= ioctl(fd1, SPI_IOC_WR_BITS_PER_WORD, &spi_bit);
    /* 设置spi传输最大速率 */
    ret |= ioctl(fd0, SPI_IOC_WR_MAX_SPEED_HZ, &st->spi_clock_speed);
    ret |= ioctl(fd1, SPI_IOC_WR_MAX_SPEED_HZ, &st->spi_clock_speed);

    if(ret != 0){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Failed to operate SPI device file.\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_BUS_FAULT;
    }

    /* 向两个片选的CHIP_ID都执行2次读操作 */
    /* 以保证accel的SPI模式被开启 */
    tx[0] = BMI088_REG_ACC_CHIP_ID | 0x80;
    ret |= SPI_Transfer(fd0, st->spi_clock_speed, tx, rx, 2);
    ret |= SPI_Transfer(fd1, st->spi_clock_speed, tx, rx, 2);
    ret |= SPI_Transfer(fd0, st->spi_clock_speed, tx, rx, 2);
    ret |= SPI_Transfer(fd1, st->spi_clock_speed, tx, rx, 2);
    if(ret < 0)return RDK_IMU_BUS_FAULT;

    /* 寻找accel的chip id */
    tx[0] = BMI088_REG_ACC_CHIP_ID | 0x80;
    ret = SPI_Transfer(fd0, st->spi_clock_speed, tx, rx, 2);
#ifdef DEBUG_INFO
    printf("[%s@%s:%d]Scan register 0x%02X from spidev1.0, ret: %d, the output is 0x%02X.\n", __FILE__, __func__, __LINE__, BMI088_REG_ACC_CHIP_ID, ret, rx[1]);
#endif
    if(ret >= 0 && rx[1] == ACC_CHIP_ID){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]The Accel Device was found in \"/dev/spidev1.0\".\n", __FILE__, __func__, __LINE__);
#endif
        st->spi_accel_fd = fd0;
    }

    ret = SPI_Transfer(fd1, st->spi_clock_speed, tx, rx, 2);
#ifdef DEBUG_INFO
    printf("[%s@%s:%d]Scan register 0x%02X from spidev1.1, ret: %d, the output is 0x%02X.\n", __FILE__, __func__, __LINE__, BMI088_REG_ACC_CHIP_ID, ret, rx[1]);
#endif
    if(ret >= 0 && rx[1] == ACC_CHIP_ID){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]The Accel Device was found in \"/dev/spidev1.1\".\n", __FILE__, __func__, __LINE__);
#endif
        st->spi_accel_fd = fd1;
    }

    /* 寻找gyro的chip id */
    tx[0] = BMI088_REG_GYRO_CHIP_ID | 0x80;
    ret = SPI_Transfer(fd0, st->spi_clock_speed, tx, rx, 2);
#ifdef DEBUG_INFO
    printf("[%s@%s:%d]Scan register 0x%02X from spidev1.0, ret: %d, the output is 0x%02X.\n", __FILE__, __func__, __LINE__, BMI088_REG_GYRO_CHIP_ID, ret, rx[1]);
#endif
    if(ret >= 0 && rx[1] == GYRO_CHIP_ID){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]The Gyro Device was found in \"/dev/spidev1.0\".\n", __FILE__, __func__, __LINE__);
#endif
        st->spi_gyro_fd = fd0;
    }

    ret = SPI_Transfer(fd1, st->spi_clock_speed, tx, rx, 2);
#ifdef DEBUG_INFO
    printf("[%s@%s:%d]Scan register 0x%02X from spidev1.1, ret: %d, the output is 0x%02X.\n", __FILE__, __func__, __LINE__, BMI088_REG_GYRO_CHIP_ID, ret, rx[1]);
#endif
    if(ret >= 0 && rx[1] == GYRO_CHIP_ID){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]The Gyro Device was found in \"/dev/spidev1.1\".\n", __FILE__, __func__, __LINE__);
#endif
        st->spi_gyro_fd = fd1;
    }

    /* 判断片选是否都有响应 */
    if(st->spi_gyro_fd != 0 && st->spi_accel_fd != 0){
        /* 扫描成功 */
        return RDK_IMU_OK;
    }

    /* 程序执行到这里说明没有找到设备 */
    /* 返回总线错误 */
#ifdef DEBUG_INFO
    printf("[%s@%s:%d]IMU was not found on spidev1.x bus.\n", __FILE__, __func__, __LINE__);
#endif

    return RDK_IMU_BUS_FAULT;
}

static enum rdk_imu_error RDK_IMU_Accel_Read(
    struct imu_state *st,
    uint8_t reg,
    uint8_t *buff,
    uint8_t len)
{
    if(len>31)return RDK_IMU_NO_SUPPORT;

    int ret = 0;

    if(st->tsmt_intf == IMU_TSMT_INTF_SPI){

        uint8_t tx[len+2];
        uint8_t rx[len+2];
        tx[0] = reg | 0x80;

        ret |= SPI_Transfer(st->spi_accel_fd, st->spi_clock_speed, tx, rx, len+2);

        if(ret < 0)return RDK_IMU_BUS_FAULT;

        for(uint8_t i=0; i<len; i++)buff[i] = rx[i+2];

        return RDK_IMU_OK;
    }
    else if(st->tsmt_intf == IMU_TSMT_INTF_I2C){
        ret |= I2C_Read_Register(st->i2c_accel_fd, st->i2c_accel_addr, reg, buff, len);

        if(ret < 0)return RDK_IMU_BUS_FAULT;

        return RDK_IMU_OK;
    }
    else return RDK_IMU_NOT_INITED;
}

static enum rdk_imu_error RDK_IMU_Gyro_Read(
    struct imu_state *st,
    uint8_t reg,
    uint8_t *buff,
    uint8_t len)
{
    if(len>31)return RDK_IMU_NO_SUPPORT;

    int ret = 0;

    if(st->tsmt_intf == IMU_TSMT_INTF_SPI){
        uint8_t tx[len+1];
        uint8_t rx[len+1];
        tx[0] = reg | 0x80;

        ret |= SPI_Transfer(st->spi_gyro_fd, st->spi_clock_speed, tx, rx, len+1);

        if(ret < 0)return RDK_IMU_BUS_FAULT;

        for(uint8_t i=0; i<len; i++)buff[i] = rx[i+1];

        return RDK_IMU_OK;
    }
    else if(st->tsmt_intf == IMU_TSMT_INTF_I2C){
        ret |= I2C_Read_Register(st->i2c_gyro_fd, st->i2c_gyro_addr, reg, buff, len);

        if(ret < 0)return RDK_IMU_BUS_FAULT;

        return RDK_IMU_OK;
    }
    else return RDK_IMU_NOT_INITED;
}

static enum rdk_imu_error RDK_IMU_Accel_Write(
    struct imu_state *st,
    uint8_t reg,
    const uint8_t data)
{
    int ret = 0;

    if(st->tsmt_intf == IMU_TSMT_INTF_SPI){
        uint8_t tx[2];
        uint8_t rx[2];
        tx[0] = reg & 0x7F;
        tx[1] = data;

        ret |= SPI_Transfer(st->spi_accel_fd, st->spi_clock_speed, tx, rx, 2);

        if(ret < 0)return RDK_IMU_BUS_FAULT;

        return RDK_IMU_OK;
    }
    else if(st->tsmt_intf == IMU_TSMT_INTF_I2C){
        ret |= I2C_Write_Register(st->i2c_accel_fd, st->i2c_accel_addr, reg, &data, 1);
        
        if(ret < 0)return RDK_IMU_BUS_FAULT;

        return RDK_IMU_OK;
    }
    else return RDK_IMU_NOT_INITED;
}

static enum rdk_imu_error RDK_IMU_Gyro_Write(
    struct imu_state *st,
    uint8_t reg,
    const uint8_t data)
{
    int ret = 0;

    if(st->tsmt_intf == IMU_TSMT_INTF_SPI){
        uint8_t tx[2];
        uint8_t rx[2];
        tx[0] = reg & 0x7F;
        tx[1] = data;

        ret |= SPI_Transfer(st->spi_gyro_fd, st->spi_clock_speed, tx, rx, 2);
        
        if(ret < 0)return RDK_IMU_BUS_FAULT;

        return RDK_IMU_OK;
    }
    else if(st->tsmt_intf == IMU_TSMT_INTF_I2C){
        ret |= I2C_Write_Register(st->i2c_gyro_fd, st->i2c_gyro_addr, reg, &data, 1);

        if(ret < 0)return RDK_IMU_BUS_FAULT;

        return RDK_IMU_OK;
    }
    else return RDK_IMU_NOT_INITED;
}

enum rdk_imu_error RDK_IMU_Accel_Pwr_Set(
    struct imu_state *st,
    enum bmi088_acc_pwr_mode pwr_mode)
{
    enum rdk_imu_error ret;
    uint8_t acc_pwr_ctrl_reg_val = 0x00;
    uint8_t acc_pwr_ctrl_reg_val_;
    uint8_t i;

    switch(pwr_mode){
        case ACC_PWR_OFF:
            acc_pwr_ctrl_reg_val |= ACC_PWR_CTRL_OFF;
            break;
        case ACC_PWR_SUSPEND:
            acc_pwr_ctrl_reg_val |= ACC_PWR_CTRL_OFF;
            break;
        case ACC_PWR_ON:
            acc_pwr_ctrl_reg_val |= ACC_PWR_CTRL_ON;
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    }

    /* 尝试3次 */
    for(i=0; i<3; i++){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Attempt %d: write 0x%02X into register 0x%02X\n", __FILE__, __func__, __LINE__, i, acc_pwr_ctrl_reg_val, BMI088_REG_ACC_PWR_CTRL);
#endif
        /* 写入 */
        ret = RDK_IMU_Accel_Write(st, BMI088_REG_ACC_PWR_CTRL, acc_pwr_ctrl_reg_val);
        if(ret!=RDK_IMU_OK)return ret;
        usleep(IMU_ACCEL_PWR_DELAY);

        /* 回读验证 */
        ret = RDK_IMU_Accel_Read(st, BMI088_REG_ACC_PWR_CTRL, &acc_pwr_ctrl_reg_val_, 1);
        if(ret!=RDK_IMU_OK)return ret;

#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Attempt %d: data 0x%02X read from register 0x%02X\n", __FILE__, __func__, __LINE__, i, acc_pwr_ctrl_reg_val_, BMI088_REG_ACC_PWR_CTRL);
#endif

        if(acc_pwr_ctrl_reg_val == acc_pwr_ctrl_reg_val_){
            /* 写入成功 */
            st->acc_pwr_mode = pwr_mode;
            return RDK_IMU_OK;
        }
    }

    return RDK_IMU_HARD_FAULT;
}

enum rdk_imu_error RDK_IMU_Gyro_Pwr_Set(
    struct imu_state *st,
    enum bmi088_gyro_lpm1 pwr_mode)
{
    int ret;
    uint8_t gyro_lpm1_val = 0x00;
    uint8_t gyro_lpm1_val_;
    uint8_t i;

    switch(pwr_mode){
        case GYRO_LPM1_NORMAL:
        case GYRO_LPM1_SUSPEND:
        case GYRO_LPM1_DEEP_SUSPEND:
            gyro_lpm1_val |= pwr_mode;
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    } 
    
    /* 尝试3次 */
    for(i=0; i<3; i++){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Attempt %d: write 0x%02X into register 0x%02X\n", __FILE__, __func__, __LINE__, i, gyro_lpm1_val, BMI088_REG_GYRO_LPM1);
#endif
        /* 写入 */
        ret = RDK_IMU_Gyro_Write(st, BMI088_REG_GYRO_LPM1, gyro_lpm1_val);            
        if(ret!=RDK_IMU_OK)return ret;
        usleep(IMU_GYRO_PWR_DELAY);

        /* 回读验证 */
        ret = RDK_IMU_Gyro_Read(st, BMI088_REG_GYRO_LPM1, &gyro_lpm1_val_, 1);
        if(ret!=RDK_IMU_OK)return ret;

#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Attempt %d: data 0x%02X read from register 0x%02X\n", __FILE__, __func__, __LINE__, i, gyro_lpm1_val_, BMI088_REG_GYRO_LPM1);
#endif

        if(gyro_lpm1_val == gyro_lpm1_val_){
            /* 写入成功 */
            st->gyro_pwr_mode = pwr_mode;
            return RDK_IMU_OK;
        }
    }

    return RDK_IMU_HARD_FAULT;
}

enum rdk_imu_error RDK_IMU_Accel_Config(
    struct imu_state *st,
    enum bmi088_acc_range range,
    enum bmi088_acc_bwp bwp,
    enum bmi088_acc_odr odr)
{
    /* 挂起模式禁止操作 */
    if(st->acc_pwr_mode != ACC_PWR_ON){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Accelerometer not activated, operation prohibited!!!\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_NO_SUPPORT;
    }

    enum rdk_imu_error ret;
    uint8_t acc_range_val = 0x00, acc_bwp_odr_val = 0x80;
    uint8_t acc_range_val_, acc_bwp_odr_val_;
    uint8_t error_code;

    switch(range){
        case ACC_RANGE_3G:
        case ACC_RANGE_6G:
        case ACC_RANGE_12G:
        case ACC_RANGE_24G:
            acc_range_val |= ACC_RANGE_MASK & ACC_RANGE(range);
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    }
    switch(bwp){
        case ACC_BWP_OSR4:
        case ACC_BWP_OSR2:
        case ACC_BWP_NORMAL:
            acc_bwp_odr_val |= ACC_BWP_MASK & ACC_BWP(bwp);
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    }
    switch(odr){
        case ACC_ODR_12_5_HZ:
        case ACC_ODR_25_HZ:
        case ACC_ODR_50_HZ:
        case ACC_ODR_100_HZ:
        case ACC_ODR_200_HZ:
        case ACC_ODR_400_HZ:
        case ACC_ODR_800_HZ:
        case ACC_ODR_1600_HZ:
            acc_bwp_odr_val |= ACC_ODR_MASK & ACC_ODR(odr);
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    }

    /* 写入 */
    ret = RDK_IMU_Accel_Write(st, BMI088_REG_ACC_RANGE, acc_range_val);
    if(ret!=RDK_IMU_OK)return ret;
    ret = RDK_IMU_Accel_Write(st, BMI088_REG_ACC_CONF, acc_bwp_odr_val);
    if(ret!=RDK_IMU_OK)return ret;

    /* 回读验证 */
    ret = RDK_IMU_Accel_Read(st, BMI088_REG_ACC_RANGE, &acc_range_val_, 1);
    if(ret!=RDK_IMU_OK)return ret;
    ret = RDK_IMU_Accel_Read(st, BMI088_REG_ACC_CONF, &acc_bwp_odr_val_, 1);
    if(ret!=RDK_IMU_OK)return ret;

    if(acc_range_val != acc_range_val_ ||
        acc_bwp_odr_val != acc_bwp_odr_val_)return RDK_IMU_HARD_FAULT;

    /* 检查配置是否有误 */
    ret = RDK_IMU_Accel_Read(st, BMI088_REG_ACC_ERR_REG, &error_code, 1);
    if(ret!=RDK_IMU_OK)return ret;
    if((error_code & ERROR_CODE_MASK) == 0x08)return RDK_IMU_ACCEL_ERROR;
    if((error_code & FATAL_ERR_MASK) == 0x01)return RDK_IMU_ACCEL_ERROR;

    /* 写入成功 */
    st->acc_range = range;
    st->acc_bwp = bwp;
    st->acc_odr = odr;

    return RDK_IMU_OK;
}

enum rdk_imu_error RDK_IMU_Gyro_Config(
    struct imu_state *st,    
    enum bmi088_gyro_range range,
    enum bmi088_gyro_bandwidth bandwidth)
{
    /* 挂起模式禁止操作 */
    if(st->gyro_pwr_mode != GYRO_LPM1_NORMAL){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]Gyroscope not activated, operation prohibited!!!\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_NO_SUPPORT;
    }

    enum rdk_imu_error ret;
    uint8_t gyro_bandwidth_val = 0x80, gyro_range_val = 0x00;
    uint8_t gyro_bandwidth_val_, gyro_range_val_;

    switch(bandwidth){
        case GYRO_ODR_2000HZ_BANDWIDTH_532HZ:
        case GYRO_ODR_2000HZ_BANDWIDTH_230HZ:
        case GYRO_ODR_1000HZ_BANDWIDTH_116HZ:
        case GYRO_ODR_400HZ_BANDWIDTH_47HZ:
        case GYRO_ODR_200HZ_BANDWIDTH_23HZ:
        case GYRO_ODR_100HZ_BANDWIDTH_12HZ:
        case GYRO_ODR_200HZ_BANDWIDTH_64HZ:
        case GYRO_ODR_100HZ_BANDWIDTH_32HZ:
            gyro_bandwidth_val |= bandwidth;
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    }
    switch(range){
        case GYRO_RANGE_2000DPS:
        case GYRO_RANGE_1000DPS:
        case GYRO_RANGE_500DPS:
        case GYRO_RANGE_250DPS:
        case GYRO_RANGE_125DPS:
            gyro_range_val |= range;
            break;
        default:
            /* 无效值 */
            return RDK_IMU_INVALID_PARAM;
    }

    /* 写入 */
    ret = RDK_IMU_Gyro_Write(st, BMI088_REG_GYRO_BANDWIDTH, gyro_bandwidth_val);
    if(ret!=RDK_IMU_OK)return ret;
    ret = RDK_IMU_Gyro_Write(st, BMI088_REG_GYRO_RANGE, gyro_range_val);
    if(ret!=RDK_IMU_OK)return ret;

    /* 回读验证 */
    ret = RDK_IMU_Gyro_Read(st, BMI088_REG_GYRO_BANDWIDTH, &gyro_bandwidth_val_, 1);
    if(ret!=RDK_IMU_OK)return ret;
    ret = RDK_IMU_Gyro_Read(st, BMI088_REG_GYRO_RANGE, &gyro_range_val_, 1);
    if(ret!=RDK_IMU_OK)return ret;

    if((gyro_bandwidth_val != gyro_bandwidth_val_) || 
        gyro_range_val != gyro_range_val_)return RDK_IMU_HARD_FAULT;

    /* 写入成功 */
    st->gyro_range = range;
    st->gyro_bandwidth = bandwidth;

    return RDK_IMU_OK;
}

enum rdk_imu_error RDK_IMU_Accel_Reset(
    struct imu_state *st)
{
    enum rdk_imu_error ret;

    ret = RDK_IMU_Accel_Write(st, BMI088_REG_ACC_SOFTRESET, BMI088_REG_ACC_SOFTRESET_CMD);

    usleep(IMU_ACCEL_RESET_DELAY);

#ifdef DEBUG_INFO
    printf("[%s@%s:%d]The accel softreset command has been sent, return: %d.\n", __FILE__, __func__, __LINE__, ret);
#endif
    
    if(ret == RDK_IMU_OK){
        st->acc_pwr_mode = ACC_PWR_OFF;

        st->acc_range = ACC_RANGE_6G;
        st->acc_bwp = ACC_BWP_NORMAL;
        st->acc_odr = ACC_ODR_100_HZ;
    }

    return ret;
}

enum rdk_imu_error RDK_IMU_Gyro_Reset(
    struct imu_state *st)
{
    enum rdk_imu_error ret;

    ret = RDK_IMU_Gyro_Write(st, BMI088_REG_GYRO_SOFTRESET, BMI088_REG_GYRO_SOFTRESET_CMD);
    
    usleep(IMU_GYRO_RESET_DELAY);

#ifdef DEBUG_INFO
    printf("[%s@%s:%d]The gyro softreset command has been sent, return: %d.\n", __FILE__, __func__, __LINE__, ret);
#endif

    if(ret == RDK_IMU_OK){
        st->gyro_pwr_mode = GYRO_LPM1_NORMAL;

        st->gyro_range = GYRO_RANGE_2000DPS;
        st->gyro_bandwidth = GYRO_ODR_2000HZ_BANDWIDTH_532HZ;
    }
    
    return ret;
}

/* return (-180.0f, 180.0f] */
static inline float angle_sub(
    float angle1, 
    float angle2)
{
    float dangle = angle1 - angle2;

    while(dangle>180.0f)dangle -= 360.0f;
    while(dangle<=-180.0f)dangle += 360.0f;

    return dangle;
} 

static inline uint64_t get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

enum rdk_imu_error RDK_IMU_Read(
    struct imu_state *st,
    struct imu_data *data)
{
    /* 挂起模式禁止操作 */
    if(st->acc_pwr_mode != ACC_PWR_ON || st->gyro_pwr_mode != GYRO_LPM1_NORMAL){
#ifdef DEBUG_INFO
        printf("[%s@%s:%d]IMU not activated, operation prohibited!!!\n", __FILE__, __func__, __LINE__);
#endif
        return RDK_IMU_NO_SUPPORT;
    }

    /* 读取所有原始数据 */
    uint8_t accel_raw_list[9];
    uint8_t gyro_raw_list[6];
    uint8_t temp_raw_list[2];
    uint64_t sys_time, gyro_time;

    gyro_time = get_timestamp_us();
    RDK_IMU_Gyro_Read(st, BMI088_REG_RATE_X_LSB, gyro_raw_list, 6);
    sys_time = get_timestamp_us();
    RDK_IMU_Accel_Read(st, BMI088_REG_ACC_X_LSB, accel_raw_list, 9);
    RDK_IMU_Accel_Read(st, BMI088_REG_TEMP_MSB, temp_raw_list, 2);

    bool inheritance; /* 是否继承原包数据 */
    /* 空包/超时不继承 */
    inheritance = data->sys_timestamp != 0 && sys_time-data->sys_timestamp < st->read_timeout;

    /* 数据解析 */
    float sensor_temp;
    float accel_3_axis_data[3];
    float gyro_3_axis_data[3];
    uint32_t sensortime;
    /* temp */
    int16_t temp_int11;
    uint16_t temp_uint11 = (temp_raw_list[0]&TEMPERATURE_HSB_MASK)<<3 | (temp_raw_list[1]&TEMPERATURE_LSB_MASK)>>5;
    if(temp_uint11 > 1023)temp_int11 = temp_uint11 - 2048;
    else temp_int11 = temp_uint11;
    sensor_temp = temp_int11 * 0.125f + 23.0f;
    /* 6-axis data */
    float accel_scale, gyro_scale;
    switch(st->acc_range){
        case ACC_RANGE_3G:
            accel_scale = 3.0f / 32768.0f;
            break; 
        case ACC_RANGE_6G:
            accel_scale = 6.0f / 32768.0f;
            break; 
        case ACC_RANGE_12G:
            accel_scale = 12.0f / 32768.0f;
            break; 
        case ACC_RANGE_24G:
            accel_scale = 24.0f / 32768.0f;
            break; 
        default:
            return RDK_IMU_INVALID_PARAM;
    }
    switch(st->gyro_range){
        case GYRO_RANGE_2000DPS:
            gyro_scale = 2000.0f / 32768.0f;
            break; 
        case GYRO_RANGE_1000DPS:
            gyro_scale = 1000.0f / 32768.0f;
            break; 
        case GYRO_RANGE_500DPS:
            gyro_scale = 500.0f / 32768.0f;
            break; 
        case GYRO_RANGE_250DPS:
            gyro_scale = 250.0f / 32768.0f;
            break; 
        case GYRO_RANGE_125DPS:
            gyro_scale = 125.0f / 32768.0f;
            break; 
        default:
            return RDK_IMU_INVALID_PARAM;
    }
    int16_t accel_3_axis_datad[3];
    int16_t gyro_3_axis_datad[3];
    for(int i=0; i<3; i++){
        accel_3_axis_datad[i] = accel_raw_list[2*i] | accel_raw_list[2*i+1]<<8;
        accel_3_axis_data[i] = -accel_3_axis_datad[i] * accel_scale;
        gyro_3_axis_datad[i] = gyro_raw_list[2*i] | gyro_raw_list[2*i+1]<<8;
        gyro_3_axis_data[i] = gyro_3_axis_datad[i] * gyro_scale;
    }
    /* Sensor time */
    sensortime = accel_raw_list[8] << 16 | accel_raw_list[7] << 8 | accel_raw_list[6]; 

    /* 温度直接赋值 */
    data->temp = sensor_temp;

    /* Gyro数据赋值 */
    /* 由于噪声,两帧之间数据大大大概率会有差异，对比可以判断数据是否ODR */
    /* 由于Gyro不配有sensortime寄存器 */
    /* 只能通过 数据变化+时间差长度 联合判断是否更新时间戳 */    
    if(data->angvel.x == gyro_3_axis_data[0] &&
        data->angvel.y == gyro_3_axis_data[1] &&
        data->angvel.z == gyro_3_axis_data[2])
    { /* 此处数据完全一致，进一步判断时间间隔是否够长 */
        /* 计算时间间隔是否大于ODR */
        uint16_t gyro_odr_time;
        switch(st->gyro_bandwidth){
            case GYRO_ODR_2000HZ_BANDWIDTH_532HZ:
            case GYRO_ODR_2000HZ_BANDWIDTH_230HZ:
                gyro_odr_time = 500;
                break;
            case GYRO_ODR_1000HZ_BANDWIDTH_116HZ:
                gyro_odr_time = 1000;
                break;
            case GYRO_ODR_400HZ_BANDWIDTH_47HZ:
                gyro_odr_time = 2500;
                break;
            case GYRO_ODR_200HZ_BANDWIDTH_23HZ:
            case GYRO_ODR_200HZ_BANDWIDTH_64HZ:
                gyro_odr_time = 5000;
                break;
            case GYRO_ODR_100HZ_BANDWIDTH_12HZ:
            case GYRO_ODR_100HZ_BANDWIDTH_32HZ:
                gyro_odr_time = 10000;
                break;
            default:
                return RDK_IMU_INVALID_PARAM;
        }
        if(gyro_time - data->angvel.timestamp > gyro_odr_time){
            data->angvel.timestamp = gyro_time;
        }
    }
    else{ /* 此处为数据存在差异的情况，直接刷新所有数据即可 */
        data->angvel.x = gyro_3_axis_data[0];
        data->angvel.y = gyro_3_axis_data[1];
        data->angvel.z = gyro_3_axis_data[2];
        data->angvel.timestamp = gyro_time;
    }
    
    /* Accel数据赋值 */
    /* Accel数据是与sensortime同步读出的，sensortime也同步存入结构体中了 */
    /* 因此直接判断是否ODR即可得到数据刷新时的sensortime */
    /* 但要注意系统时间戳和sensortime的融合 */
    /* 其主要思想类似互补滤波 */
    /* 对于ODR数据，系统时间戳不准，但长期下来低频稳定性高 */
    /* 对于ODR数据，sensortime精度低，但短期高频精度比系统时间戳高 */

    if(inheritance){ // 继承帧的accel时间戳处理
        /* 先判断是否ODR */
        if(data->accel.x != accel_3_axis_data[0] ||
            data->accel.y != accel_3_axis_data[1] ||
            data->accel.z != accel_3_axis_data[2]){ /* 有数据变化 */
            /* 计算两帧数据的sensor时间差（不缩放） */
            int32_t delta_sensortime_signed = sensortime - data->imu_sensortime;
            /* 处理溢出的sensortime原始数据 */
            while(delta_sensortime_signed < 0)delta_sensortime_signed += 0xFFFFFF; /* 24bit */
            /* 缩放sensor时间差 */
            uint32_t delta_sensortime = delta_sensortime_signed * st->sensor_time_scale_numerator / st->sensor_time_scale_denominator;
            /* 计算sys时间差 */
            uint32_t delta_systime = sys_time - data->accel.timestamp;
            /* 根据宏权重赋给结构体中accel的时间戳变量 */
            data->accel.timestamp += (uint64_t)(
                delta_systime * (1 - st->time_sync_weight) + 
                delta_sensortime * st->time_sync_weight);
        }
        else{
            /* 仅更新sys_timestamp */
            data->sys_timestamp = sys_time;
        }
        /* 更新sensortime */
        data->imu_sensortime = sensortime;
    }
    else{ // 非继承帧的时间戳处理
        data->accel.timestamp = sys_time;
        data->sys_timestamp = sys_time;
        data->imu_sensortime = sensortime;
    }
    /* 最后赋值更新三轴数据 */
    data->accel.x = accel_3_axis_data[0];
    data->accel.y = accel_3_axis_data[1];
    data->accel.z = accel_3_axis_data[2];

    return RDK_IMU_OK;
}
