# ==============================================================================
# 项目构建配置文件 (Makefile)
# ==============================================================================
# 项目名称    : rdk_imu_module
# 描述        : 用于编译、测试、清理和管理RDK_IMU_Module的自动化脚本。
# 创建日期    : 2026-1-19
# 最后修改    : 2026-1-19
# 作者/维护者 : xiaoye.zhang@Leaf from D-Robotics.
# ==============================================================================
PROJECT_NAME = rdk_imu_module

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin
INC_DIR = include
CYTHON_DIR = Py_RDK_IMU
SAMPLE_DIR = examples

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0

CC = gcc
CFLAGS = -Wall -Wextra -I$(INC_DIR)
LDFLAGS = -L$(LIB_DIR)
LDLIBS = -l$(PROJECT_NAME)

.PHONY: all directories obj obj-Wo-gpio static static-Wo-gpio dynamic dynamic-Wo-gpio sample sample-Wo-gpio cython clean

all: directories obj static dynamic sample sample-Wo-gpio cython

directories:
	@echo "[$(PROJECT_NAME) Makefile]Make directories."
	mkdir -p $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

obj: directories $(OBJ_DIR)/$(PROJECT_NAME)_data.o $(OBJ_DIR)/$(PROJECT_NAME)_gpio.o

obj-Wo-gpio: directories $(OBJ_DIR)/$(PROJECT_NAME)_data.o

$(OBJ_DIR)/$(PROJECT_NAME)_data.o: $(SRC_DIR)/$(PROJECT_NAME)_data.c
	$(CC) $(CFLAGS) -Wno-gpio -c $< -o $@ -fPIC

$(OBJ_DIR)/$(PROJECT_NAME)_gpio.o: $(SRC_DIR)/$(PROJECT_NAME)_gpio.c
	$(CC) $(CFLAGS) -Wno-gpio -c $< -o $@ -fPIC

static: obj
	@echo "[$(PROJECT_NAME) Makefile]Make static libraries."
	ar rcs $(LIB_DIR)/lib$(PROJECT_NAME)_data.a $(OBJ_DIR)/$(PROJECT_NAME)_data.o
	ar rcs $(LIB_DIR)/lib$(PROJECT_NAME)_gpio.a $(OBJ_DIR)/$(PROJECT_NAME)_gpio.o

static-Wo-gpio: obj-Wo-gpio
	@echo "[$(PROJECT_NAME) Makefile]Make static libraries without gpio."
	ar rcs $(LIB_DIR)/lib$(PROJECT_NAME)_data.a $(OBJ_DIR)/$(PROJECT_NAME)_data.o

dynamic: obj
	@echo "[$(PROJECT_NAME) Makefile]Make dynamic link libraries."

	$(CC) -shared -Wl,-soname,$(LIB_DIR)/lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR) \
	$(OBJ_DIR)/$(PROJECT_NAME)_data.o \
	-o $(LIB_DIR)/lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

	cd $(LIB_DIR) && \
		ln -sf lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) \
		       lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR) && \
		ln -sf lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR) \
		       lib$(PROJECT_NAME)_data.so

	$(CC) -shared -Wl,-soname,$(LIB_DIR)/lib$(PROJECT_NAME)_gpio.so.$(VERSION_MAJOR) \
	$(OBJ_DIR)/$(PROJECT_NAME)_gpio.o \
	-o $(LIB_DIR)/lib$(PROJECT_NAME)_gpio.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

	cd $(LIB_DIR) && \
		ln -sf lib$(PROJECT_NAME)_gpio.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) \
		       lib$(PROJECT_NAME)_gpio.so.$(VERSION_MAJOR) && \
		ln -sf lib$(PROJECT_NAME)_gpio.so.$(VERSION_MAJOR) \
		       lib$(PROJECT_NAME)_gpio.so

dynamic-Wo-gpio: obj-Wo-gpio
	@echo "[$(PROJECT_NAME) Makefile]Make dynamic link libraries without gpio."
	
	$(CC) -shared -Wl,-soname,$(LIB_DIR)/lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR) \
	$(OBJ_DIR)/$(PROJECT_NAME)_data.o \
	-o $(LIB_DIR)/lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

	cd $(LIB_DIR) && \
		ln -sf lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) \
		       lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR) && \
		ln -sf lib$(PROJECT_NAME)_data.so.$(VERSION_MAJOR) \
		       lib$(PROJECT_NAME)_data.so

sample: $(SAMPLE_DIR)/imu_test.c static dynamic
	@echo "[$(PROJECT_NAME) Makefile]Compile sample program."
	$(CC) $(CFLAGS) $(SAMPLE_DIR)/imu_test.c -o $(BIN_DIR)/imu_test -L$(LIB_DIR) -l$(PROJECT_NAME)_data -l$(PROJECT_NAME)_gpio -lm -lwiringPi -lwiringPiDev

sample-Wo-gpio: $(SAMPLE_DIR)/imu_test_without_gpio.c static-Wo-gpio dynamic-Wo-gpio
	@echo "[$(PROJECT_NAME) Makefile]Compile sample program without gpio."
	$(CC) $(CFLAGS) $(SAMPLE_DIR)/imu_test_without_gpio.c -o $(BIN_DIR)/imu_test_without_gpio -L$(LIB_DIR) -l$(PROJECT_NAME)_data -lm

cython: $(CYTHON_DIR)/Py_RDK_IMU.pyx $(CYTHON_DIR)/setup.py $(SRC_DIR)/$(PROJECT_NAME)_data.c $(SRC_DIR)/$(PROJECT_NAME)_gpio.c
	cd $(CYTHON_DIR) && sudo python3 setup.py build_ext --inplace

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
	rm -rf $(CYTHON_DIR)/build
	rm -rf $(CYTHON_DIR)/RDK_Pyimu.c
	rm -rf $(CYTHON_DIR)/*.so
