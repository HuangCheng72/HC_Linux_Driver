# 指定内核构建目录
KERNELDIR := /home/hc/orangepi_h3_linux/OrangePi-Kernel/linux-4.9

# 当前目录
PWD := $(shell pwd)

# 交叉编译器前缀
CROSS_COMPILE := arm-linux-gnueabi-

# 架构
ARCH := arm

# 内核模块目标
obj-m := dma_test.o

# 默认目标
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

# 清理目标
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

.PHONY: all clean
