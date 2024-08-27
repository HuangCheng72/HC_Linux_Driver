# 指定内核版本
KERNEL_VERSION := $(shell uname -r)

# 指定内核构建目录
KERNELDIR := /lib/modules/$(KERNEL_VERSION)/build

# 当前目录
PWD := $(shell pwd)

# 内核模块目标
obj-m := ring_buf.o

# 默认目标
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

# 清理目标
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

.PHONY: all clean
