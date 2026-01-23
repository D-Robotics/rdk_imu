# **RDK_IMU_Module_us**
## ———— *Driver RDK IMU Module in User Space* ————
&emsp;基于用户空间的RDK IMU Module软件包，完全不依赖iio相关驱动，仅使用linux标准通用I2C与SPI驱动<br>
&emsp;**Version: 1.0.0**

## **一、编译说明**

### **1.1.依赖说明**

项目分为两个部分：<br>

1. **rdk_imu_module_data:** 核心部分，用于调用Linux通用SPI与I2C驱动与IMU通信，并进行数据解析与时间同步，编译需`-lm`链接math<br>

2. **rdk_imu_module_gpio:** 次要部分，用于驱动**RDK IMU Module**载板上的GPIO设备（LED、蜂鸣器和1-wire温度传感器），依赖**RDK wiringPi**，需要按照以下教程进行**RDK wiringPi**的安装：[在旭日X3派上移植的 WiringPi](https://gitee.com/xin03liu/WiringPi)，并在编译时`-lwiringPi`链接**wiring Pi**<br>

- 不安装**RDK wiringPi**也可以使用**rdk_imu_module_data**的C部分内容，但不能使用**rdk_imu_module_gpio**和Python部分内容<br>

### **1.2.Makefile使用**

&emsp;项目目录下编写了Makefile，可以通过make快速编译项目，`cd /path/to/RDK_IMU_Module_us`来到项目目录，`make`或`make all`编译所有项目，或者：<br>

1. `make static`: 编译完整功能的`.a`静态链接库到`./lib`路径下<br>

2. `make static-Wo-gpio`: 仅编译**rdk_imu_module_data**部分内容的`.a`静态链接库到`./lib`路径下<br>

3. `make dynamic`: 编译完整功能的`.so`动态链接库到`./lib`路径下<br>

4. `make dynamic-Wo-gpio`: 仅编译**rdk_imu_module_data**部分内容的`.so`动态链接库到`./lib`路径下<br>

5. `make sample`: 编译完整功能的测试用例可执行文件到`./bin`路径下<br>

6. `make sample-Wo-gpio`: 仅编译**rdk_imu_module_data**部分内容的测试用例可执行文件到`./bin`路径下<br>

7. `make cython`: 编译Cython部分内容<br>

8. `make clean`: 清理所有内容<br>

## **二、Python-Sample快速体验**

### **2.1.环境准备**

#### **2.1.1.wiringPi**安装

&emsp;按照**1.1.依赖说明**中的链接安装`RDK wiringPi`，使用命令`gpio readall`检查，输出以下内容：<br>

```text
root@ubuntu:~/RDK_IMU_Module_us# gpio readall
 +-----+-----+-----------+--RDK X5--+-----------+-----+-----+
 | BCM | xPi |    Name   | Physical |   Name    | xPi | BCM |
 +-----+-----+-----------+----++----+-----------+-----+-----+
 |     |     |      3.3v |  1 || 2  | 5v        |     |     |
 |   2 | 390 |     SDA.5 |  3 || 4  | 5v        |     |     |
 |   3 | 389 |     SCL.5 |  5 || 6  | 0v        |     |     |
 |   4 | 420 | I2S1_MCLK |  7 || 8  | TxD.1     | 383 | 14  |
 |     |     |        0v |  9 || 10 | RxD.1     | 384 | 15  |
 |  17 | 380 |  GPIO. 17 | 11 || 12 | I2S1_BCLK | 421 | 18  |
 |  27 | 379 |  GPIO. 27 | 13 || 14 | 0v        |     |     |
 |  22 | 388 |  GPIO. 22 | 15 || 16 | GPIO. 23  | 382 | 23  |
 |     |     |      3.3v | 17 || 18 | GPIO. 24  | 402 | 24  |
 |  10 | 398 | SPI1_MOSI | 19 || 20 | 0v        |     |     |
 |   9 | 397 | SPI1_MISO | 21 || 22 | GPIO. 25  | 387 | 25  |
 |  11 | 395 | SPI1_SCLK | 23 || 24 | SPI1_CSN1 | 394 | 8   |
 |     |     |        0v | 25 || 26 | SPI1_CSN0 | 396 | 7   |
 |   0 | 355 |     SDA.0 | 27 || 28 | SCL.0     | 354 | 1   |
 |   5 | 399 |   GPIO. 5 | 29 || 30 | 0v        |     |     |
 |   6 | 400 |   GPIO. 6 | 31 || 32 | PWM6      | 356 | 12  |
 |  13 | 357 |      PWM7 | 33 || 34 | 0v        |     |     |
 |  19 | 422 | I2S1_LRCK | 35 || 36 | GPIO. 16  | 381 | 16  |
 |  26 | 401 |   GPIO.26 | 37 || 38 | I2S1_DIN  | 423 | 20  |
 |     |     |        0v | 39 || 40 | I2S1_DOUT | 424 | 21  |
 +-----+-----+-----------+----++----+-----------+-----+-----+
 | BCM | xPi |    Name   | Physical |   Name    | xPi | BCM |
 +-----+-----+-----------+--RDK X5--+-----------+-----+-----+
```

#### **2.1.2.动态链接库编译**

&emsp;项目自带一个编译好的、可用于python的`.so`动态链接库文件，位于`RDK_IMU_Module_us/Py_RDK_IMU/rdkimu.cpython-310-aarch64-linux-gnu.so`，如果确认`RDK wiringPi`已正确安装，可以直接运行sample<br>

&emsp;如果需要重新编译Cython动态链接库，回到Makefile目录下`make cython`即可<br>

### **2.2.sample执行**

&emsp;来到`RDK_IMU_Module_us/Py_RDK_IMU`目录下，`python3 test_pyimu.py`执行测试用例，终端将依次输出imu地址扫描结果和imu初始化结果，随后伴随蜂鸣器的响声，**RDK IMU Module**载板上的3个LED依次闪烁，然后终端输出板载温度，最后程序将会循环读取并打印IMU的6轴数据和时间戳、温度数据<br>

## **三、C-Samples快速体验**

### **3.1.完整sample运行**

&emsp;`cd /path/to/RDK_IMU_Module_us`来到项目目录下，通过`make sample`编译测试用例，确认编译成功后可直接运行`./bin/imu_test`文件，测试用例将会进行IMU设备的扫描与初始化，并驱动**RDK IMU Module**载板发出声光提示，输出以下内容：<br>

```text
root@ubuntu:~/RDK_IMU_Module_us# ./bin/imu_test 
[examples/imu_test.c]Successfully detected the BMI088 device.
[examples/imu_test.c]RDK_IMU_Accel_Reset 0
[examples/imu_test.c]RDK_IMU_Gyro_Reset 0
[examples/imu_test.c]IMU initialization successful 0
[examples/imu_test.c]GPIO initialization successful 0
[examples/imu_test.c]The 0th time obtaining the ambient temperature is 31.312500 Celsius.
[examples/imu_test.c]The 1th time obtaining the ambient temperature is 31.312500 Celsius.
[examples/imu_test.c]The 2th time obtaining the ambient temperature is 31.375000 Celsius.
```

&emsp;然后清屏，输出IMU数据，如下：<br>

```text
[imu data] accel: -0.015,  -0.012,  -1.008, timestamp: 1768881430201181
[imu data]  gyro:  0.153,   0.580,   0.443, timestamp: 1768881430200252
[temp data] temp: 28.875000
```

### **3.2.仅运行核心示例**

&emsp;不安装、不链接wiringPi，仅运行核心示例，只需要使用`make sample-Wo-gpio`编译不带gpio的测试用例，将会`./bin/imu_test_without_gpio`文件，运行后程序同样会进行IMU设备的扫描与初始化，但没有声光提示，然后清屏，循环打印IMU数据<br>

## **四、C接口介绍**

&emsp;`RDK_IMU_Module_us/include/rdk_imu_module.h`中定义了驱动imu的所有函数和枚举类型，`RDK_IMU_Module_us/include/bmi088_regs.h`中则定义了BMI088芯片的所有寄存器信息以及状态枚举<br>

### **4.1.错误枚举**

&emsp;`enum rdk_imu_error{}`枚举了所有的错误类型，该枚举类型作为大部分函数的返回值，值`RDK_IMU_OK`为无错误<br>

### **4.2.data接口说明**

- `struct imu_state`:<br>
- IMU状态结构体，是所有imu data相关函数的重要参数，记录了IMU模块的完整状态（不含中断）<br>
<br>

- `struct imu_data`:<br>
- IMU数据结构体，用于格式化存储imu数据包的完整数据<br>
<br>

- `struct imu_state RDK_IMU_Get_Initial_State()`:<br>
- 初始化结构体`imu_state`的标准方式<br>
    - `return`: 默认状态的`imu_state`结构体，使用一个`struct imu_state`类型变量来接收它<br>
<br>

- `struct imu_data RDK_IMU_Get_Initial_Data()`: 初始化结构体`imu_data`的标准方式<br>
    - `return`: 默认状态的`imu_data`结构体，使用一个`struct imu_data`类型变量来接收它<br>
<br>

- `enum rdk_imu_error RDK_IMU_All_Device_Scan(`
- &emsp;`struct imu_state *st)`:<br> 
- IMU设备扫描函数,支持I2C/SPI自动检测，支持I2C全总线全地址自动扫描<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Accel_Pwr_Set(`
- &emsp;`struct imu_state *st,`
- &emsp;`enum bmi088_acc_pwr_mode pwr_mode)`:<br>
- 加速度计电源模式设置<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `pwr_mode`: `enum bmi088_acc_pwr_mode`类型，可选值如下：<br>
        - `ACC_PWR_OFF`: 关闭电源<br>
        - `ACC_PWR_SUSPEND`: 挂起<br>
        - `ACC_PWR_ON`: 启动<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Gyro_Pwr_Set(`<br>
- &emsp;`struct imu_state *st,`<br>
- &emsp;`enum bmi088_gyro_lpm1 pwr_mode)`:<br>
- 陀螺仪电源模式设置<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `pwr_mode`: `enum bmi088_gyro_lpm1`类型，可选值如下：<br>
        - `GYRO_LPM1_NORMAL`: 启动<br>
        - `GYRO_LPM1_SUSPEND`: 挂起<br>
        - `GYRO_LPM1_DEEP_SUSPEND`: 深度挂起<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Accel_Config(`<br>
- &emsp;`struct imu_state *st,`<br>
- &emsp;`enum bmi088_acc_range range,`<br>
- &emsp;`enum bmi088_acc_bwp bwp,`<br>
- &emsp;`enum bmi088_acc_odr odr)`<br>
- 加速度计配置<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `range`: 加速度计量程，`bmi088_acc_range`类型，可选值如下：<br>
        - `ACC_RANGE_3G`<br>
        - `ACC_RANGE_6G`<br>
        - `ACC_RANGE_12G`<br>
        - `ACC_RANGE_24G`<br>
    - `bwp`: 加速度计滤波设置，`bmi088_acc_bwp`类型，可选值如下：<br>
        - `ACC_BWP_OSR4`<br>
        - `ACC_BWP_OSR2`<br>
        - `ACC_BWP_NORMAL`<br>
    - `odr`: 加速度计输出频率设置，`bmi088_acc_odr`类型，可选值如下：<br>
        - `ACC_ODR_12_5_HZ`<br>
        - `ACC_ODR_25_HZ`<br>
        - `ACC_ODR_50_HZ`<br>
        - `ACC_ODR_100_HZ`<br>
        - `ACC_ODR_200_HZ`<br>
        - `ACC_ODR_400_HZ`<br>
        - `ACC_ODR_800_HZ`<br>
        - `ACC_ODR_1600_HZ`<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Gyro_Config(`<br>
- &emsp;`struct imu_state *st, `<br>
- &emsp;`enum bmi088_gyro_range range,`<br>
- &emsp;`enum bmi088_gyro_bandwidth bandwidth)`:<br>
- 陀螺仪配置<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `range`: 陀螺仪量程，`bmi088_gyro_range`类型，可选值如下：<br>
        - `GYRO_RANGE_2000DPS`<br>
        - `GYRO_RANGE_1000DPS`<br>
        - `GYRO_RANGE_500DPS`<br>
        - `GYRO_RANGE_250DPS`<br>
        - `GYRO_RANGE_125DPS`<br>
    - `bandwidth`: 陀螺仪ODR与带宽，`bmi088_gyro_bandwidth`类型，可选值如下：<br>
        - `GYRO_ODR_2000HZ_BANDWIDTH_532HZ`<br>
        - `GYRO_ODR_2000HZ_BANDWIDTH_230HZ`<br>
        - `GYRO_ODR_1000HZ_BANDWIDTH_116HZ`<br>
        - `GYRO_ODR_400HZ_BANDWIDTH_47HZ`<br>
        - `GYRO_ODR_200HZ_BANDWIDTH_23HZ`<br>
        - `GYRO_ODR_100HZ_BANDWIDTH_12HZ`<br>
        - `GYRO_ODR_200HZ_BANDWIDTH_64HZ`<br>
        - `GYRO_ODR_100HZ_BANDWIDTH_32HZ`<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Accel_Reset(`<br>
- &emsp;`struct imu_state *st)`:<br>
- 加速度计软件复位<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Gyro_Reset(`<br>
- &emsp;`struct imu_state *st)`:<br>
- 陀螺仪软件复位<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `return`: `enum rdk_imu_error`错误码<br>
    - 当imu工作在i2c模式下时，该函数无法成功使能陀螺仪复位，但可以使用`DK_IMU_Gyro_Pwr_Set`函数让陀螺仪进入深度挂起模式，以达到软件复位的作用<br>
<br>

- `enum rdk_imu_error RDK_IMU_Read(`
- &emsp;`struct imu_state *st,`
- &emsp;`struct imu_data *data)`:
- IMU数据读取<br>
    - `*st`: `struct imu_state`类型指针<br>
    - `*data`: `struct imu_data`类型指针<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

### **4.3.gpio接口说明**

- `enum rdk_imu_error RDK_IMU_GPIO_Init()`:<br>
- GPIO初始化<br>
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_GPIO_Enable(`<br>
- &emsp;`enum rdk_imu_gpio_sel gpio_sel)`:<br>
- 使能选定的io设备<br>
    - `gpio_sel`: io管脚选择掩码
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_GPIO_Disable(`<br>
- &emsp;`enum rdk_imu_gpio_sel gpio_sel)`:<br>
- 关闭选定的io设备<br>
    - `gpio_sel`: io管脚选择掩码
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

- `enum rdk_imu_error RDK_IMU_Get_DS18B20_Temp(`<br>
- &emsp;`float *temp)`:<br>
- 虚拟1-wire总线向板载温度传感器获取温度数据
    - `*temp`: 浮点型温度数据指针
    - `return`: `enum rdk_imu_error`错误码<br>
<br>

## **五、Python接口介绍**

&emsp;在`RDK_IMU_Module_us/Py_RDK_IMU/Py_RDK_IMU.pyx`中，使用了Cython混编将`RDK_IMU_Module_us/src/rdk_imu_module_data.c`和`RDK_IMU_Module_us/src/rdk_imu_module_gpio.c`的实现封装成了Python的API，为了保持接口一致性，Cython封装中不将data和gpio两个模块分开，合并为同一个库中的不同类<br>

### **5.1.RDK_IMU类介绍**

#### **5.1.1.构造函数**

&emsp;RDK_IMU类的构造函数中进行了内部成员imu_st和imu_dt两个结构体的初始化<br>

#### **5.1.2.设备扫描函数：`RDK_IMU.Device_Scan()`**
&emsp;无参方法，返回一个具有2个元素的元组:<br>
- `return[0]`: 布尔值，是否成功扫描到设备<br>
- `return[1]`: 设备地址信息字典<br>

#### **5.1.3.软件复位函数：`RDK_IMU.Accel_Reset()`/`RDK_IMU.Gyro_Reset()`**
&emsp;无参方法，返回布尔值，表示软件复位是否成功<br>
&emsp;其中，`RDK_IMU.Gyro_Reset()`继承了C实现的特性：在i2c模式下无法正确复位<br>

#### **5.1.4.电源设置函数：`RDK_IMU.Set_Pwr_Mode(accel_pwr, gyro_pwr)`**
&emsp;`accel_pwr`参数需检查类型是否为`Accel_Pwr`枚举类，枚举如下：<br>
```python
@unique
class Accel_Pwr(Enum):
    pwr_on = "on"
    pwr_suspend = "suspend"
    pwr_off = "off"
```
&emsp;`gyro_pwr`参数需检查类型是否为`Gyro_Pwr`枚举类，枚举如下：<br>
```python
@unique
class Gyro_Pwr(Enum):
    pwr_on = "on"
    pwr_suspend = "suspend"
    pwr_deepsuspend = "deepsuspend"
```
&emsp;返回值为布尔值，表示是否设置成功<br>

#### **5.1.5.加速度计配置函数:`RDK_IMU.Accel_Config()`**

&emsp;可以无参调用，未指定的参数将选择默认值<br>

&emsp;可选参数-`range`:加速度量程，类型检查为`Accel_Range`枚举类，枚举如下：<br>
```python
@unique
class Accel_Range(Enum):
    _3g = 3.0
    _6g = 6.0
    _12g = 12.0
    _24g = 24.0 # 默认值
```

&emsp;可选参数-`bwp`:加速度滤波设置，类型检查为`Accel_Bwp`枚举类，枚举如下：<br>
```python
@unique
class Accel_Bwp(Enum):
    osr4 = "osr4"
    osr2 = "osr2"
    normal = "normal" # 默认值
```

&emsp;可选参数-`odr`:加速度滤波设置，类型检查为`Accel_Odr`枚举类，枚举如下：<br>
```python
@unique
class Accel_Odr(Enum):
    _12_5hz = 12.5
    _25hz = 25.0
    _50hz = 50.0
    _100hz = 100.0
    _200hz = 200.0
    _400hz = 400.0
    _800hz = 800.0
    _1600hz = 1600.0 # 默认值
```

&emsp;返回值为布尔值，表示是否设置成功<br>

&emsp;例如，设置加速度计量程为6g、bwp为osr2、odr为200hz：<br>
```python
import rdkimu

imu = rdkimu.RDK_IMU()
...
...
ret = imu.Accel_Config(
    range=rdkimu.Accel_Range._6g,
    bwp=rdkimu.Accel_Bwp.osr2,
    odr=rdkimu.Accel_Odr._200hz)

if ret:
    # 设置成功
    pass
else:
    pass
```

#### **5.1.6.陀螺仪配置函数:`RDK_IMU.Gyro_Config()`**

&emsp;可以无参调用，未指定的参数将选择默认值<br>

&emsp;可选参数-`range`:陀螺仪量程，类型检查为`Gyro_Range`枚举类，枚举如下：<br>
```python
@unique
class Gyro_Range(Enum):
    _2000dps = 2000.0 # 默认值
    _1000dps = 1000.0
    _500dps = 500.0
    _250dps = 250.0
    _125dps = 125.0
```

&emsp;可选参数-`odr_bwp`:陀螺仪ODR与滤波设置，类型检查为`Gyro_Odr_Bwp`枚举类，枚举如下：<br>
```python
@unique
class Gyro_Odr_Bwp(Enum):
    _2000hz_532hz = (2000.0, 532.0) # 默认值
    _2000hz_230hz = (2000.0, 230.0)
    _1000hz_116hz = (1000.0, 116.0)
    _400hz_47hz = (400.0, 47.0)
    _200hz_23hz = (200.0, 23.0)
    _100hz_12hz = (100.0, 12.0)
    _200hz_64hz = (200.0, 64.0)
    _100hz_32hz = (100.0, 32.0)
```

&emsp;返回值为布尔值，表示是否设置成功<br>

&emsp;例如，设置陀螺仪量程为500dps、odr为400hz、带宽为47hz：<br>
```python
import rdkimu

imu = rdkimu.RDK_IMU()
...
...
ret = imu.Accel_Config(
    range=rdkimu.Gyro_Range._500dps,
    odr_bwp=rdkimu.Gyro_Odr_Bwp._400hz_47hz)

if ret:
    # 设置成功
    pass
else:
    pass
```

#### **5.1.7.IMU状态获取函数:`RDK_IMU.Get_IMU_Info()`**

&emsp;无参方法，返回一个包含：加速度计/陀螺仪电源模式、量程、ODR频率、滤波设置等信息的字典<br>

#### **5.1.8.IMU数据更新函数:`RDK_IMU.Data_Update()`**

&emsp;无参方法，返回值为二元素元组，说明如下：<br>
- return[0]: bool值，是否更新成功<br>
- return[1]: 错误码<br>

#### **5.1.9.IMU数据包读取函数:`RDK_IMU.Data_Read()`**

&emsp;无参方法，返回一个包含IMU6轴信息和温度、时间戳信息的字典<br>

### **5.2.RDK_IMU_GPIO类介绍**

#### **5.2.1.构造函数**

&emsp;`RDK_IMU_GPIO`类实例化时会在构造函数中完成GPIO设备的初始化，如果初始化设备将抛出`RuntimeError`错误

#### **5.2.2.板载温度读取函数：`RDK_IMU_GPIO.Get_Board_Temp()`**

&emsp;无参方法，该方法调用软件1-wire总线读取板载温度传感器数据并解析，返回值为二元元组，说明如下：<br>
- `return[0]`: bool值，是否读取成功<br>
- `return[1]`: 浮点数，板载温度值，单位为摄氏度<br>

#### **5.2.3.GPIO控制函数：`RDK_IMU_GPIO.GPIO_Ctrl()`**

&emsp;该方法可以无参调用，但是不会发生任何实际操作，因为所有参数的默认值均为“保持IO状态不变”<br>

&emsp;函数拥有4个形参：`led_r_st`、`led_b_st`、`led_g_st`和`bell_st`，类型检查均为`GPIO_State`枚举类，定义如下：<br>
```python
@unique
class GPIO_State(Enum):
    _open = "open"
    _close = "close"
    _hold_on = "hold on"
```

&emsp;通过指定参数，可以控制任一GPIO设备启动、关闭、或是保持，例如，控制红色LED关闭，蜂鸣器启动，其他设备保持：<br>
```python
import rdkimu

imu_gpio = rdkimu.RDK_IMU_GPIO()
...
...
imu_gpio.GPIO_Ctrl(led_r_st=GPIO_State._close, bell_st=GPIO_State._open)

```

### **5.3.其他**

&emsp;其他方法函数介绍请查阅`.pyx`文件中的具体实现<br>

&emsp;**！！！二次开发前务必参考测试用例！！！**<br>

## **六、版本信息**
- 项目发布时间：2026-1-19
- 项目版本：v1.0.0
- 版本时间：2026-1-19
- 项目最后更改时间：2026-1-20
- README.md最后更改时间：2026-1-20

## ———— *END* ————