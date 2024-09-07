//
// Created by huangcheng on 2024/9/5.
//

/*
 * Copyright (C) 2024 huangcheng
 * Author: huangcheng <huangcheng20000702@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * 作用：这是内核模块开发的必备头文件。它包含了一些定义和宏，用于内核模块的加载与卸载。
 * 常用函数：module_init(), module_exit(), MODULE_LICENSE(), MODULE_AUTHOR()等。
 */
#include <linux/module.h>

/*
 * 作用：提供平台设备（Platform Device）相关的API。平台设备是那些通过内存映射I/O与CPU进行通信的外设。
 * 该头文件主要用于注册、移除和操作平台设备。
 * 常用函数：platform_driver_register(), platform_driver_unregister(), platform_get_resource()等。
 */
#include <linux/platform_device.h>

/*
 * 作用：提供与设备树（Device Tree）相关的操作接口。设备树是描述硬件配置的文件，通过设备树可以让内核识别和配置硬件设备。
 * 常用函数：of_match_device(), of_find_node_by_name(), of_property_read_u32()等。
 */
#include <linux/of_device.h>

/*
 * 作用：提供对设备时钟的控制，允许驱动程序启用、禁用时钟，并调整时钟频率等。
 * 时钟资源通常与SoC中的外设（如DMA控制器）相关联，驱动需要在使用外设前使能时钟。
 * 常用函数：clk_get(), clk_enable(), clk_disable(), clk_put()等。
 */
#include <linux/clk.h>

/*
 * 作用：提供内核中的延时操作。它支持微秒（us）、毫秒（ms）级别的延时，适用于需要等待硬件状态变化的场景。
 * 常用函数：mdelay(), udelay(), msleep(), usleep_range()等。
 */
#include <linux/delay.h>

/*
 * 作用：提供设备复位的控制接口。用于将设备硬件复位到初始状态。
 * 复位控制器一般与硬件控制器相关联，通过复位控制器驱动可以对设备进行复位。
 * 常用函数：reset_control_get(), reset_control_deassert(), reset_control_assert()等。
 */
#include <linux/reset.h>

/*
 * 作用：提供DMA引擎（DMA Engine）的通用接口，用于实现设备与内存之间的高效数据传输。
 * 该接口抽象了DMA传输的操作，支持内存到设备、设备到内存等各种DMA传输模式。
 * 常用函数：dma_request_chan(), dmaengine_prep_slave_single(), dmaengine_submit()等。
 */
#include <linux/dmaengine.h>

/*
 * 作用：提供DMA内存池管理，用于分配和管理小块的DMA缓冲区。DMA内存池可以减少内存碎片，提高DMA传输的效率。
 * 常用函数：dma_pool_create(), dma_pool_alloc(), dma_pool_free(), dma_pool_destroy()等。
 */
#include <linux/dmapool.h>

/*
 * 作用：提供与DMA映射相关的操作接口。驱动程序可以通过该接口将内存映射为DMA地址，供DMA控制器使用。
 * 常用函数：dma_map_single(), dma_unmap_single(), dma_alloc_coherent(), dma_free_coherent()等。
 */
#include <linux/dma-mapping.h>

/*
 * 作用：提供中断处理相关的接口，用于注册、注销中断处理函数，以及处理设备中断。
 * 当外设（如DMA控制器）触发中断时，内核会调用驱动程序注册的中断处理函数。
 * 常用函数：request_irq(), free_irq(), disable_irq(), enable_irq()等。
 */
#include <linux/interrupt.h>

/*
 * 作用：用于解析设备树中的DMA控制器节点，并将其映射到驱动中。该接口提供了设备树与DMA引擎的桥接功能。
 * 常用函数：of_dma_request_slave_channel(), of_dma_xlate_by_chan_id()等。
 */
#include <linux/of_dma.h>

/*
 * 作用：提供内核内存分配和释放的相关接口，适用于小块内存的动态分配。
 * 常用函数：kmalloc(), kfree(), kzalloc(), kcalloc()等。
 */
#include <linux/slab.h>

/*
 * 作用：定义了一些基础的数据类型，如u32、u64、s32、bool等，方便在驱动中使用标准化的数据类型。
 */
#include <linux/types.h>

/*
 * 作用：虚拟DMA通道的抽象层，提供对DMA通道的封装。virt-dma简化了DMA通道的管理，支持虚拟通道的调度和控制。
 * 该头文件直接来自内核源代码
 */
#include "/home/hc/orangepi_h3_linux/OrangePi-Kernel/linux-4.9/drivers/dma/virt-dma.h"

// 以下是DMA控制器驱动用到的诸多宏定义

#define DEVICE_NAME "hc-opi-one-dma"    // 设备名
#define CLASS_NAME "dma"                // 设备类名

// 以下来自于Allwinner_H3_Datasheet_v1.2.pdf（以下简称技术手册），p191的4.11.2.3 DMA Descriptor

#define DMA_DESCRIPTOR_END_ADDRESS  0xFFFFF800  // DMA 描述符链结束标志，表示当前包为最后一个包

// 以下来自于技术手册p192-p193的列表4.11.3 DMA Register List

// DMA IRQ 寄存器（中断请求寄存器）
#define DMA_IRQ_EN_REG0_OFFSET      0x00    // DMA IRQ 使能寄存器0
#define DMA_IRQ_EN_REG1_OFFSET      0x04    // DMA IRQ 使能寄存器1
#define DMA_IRQ_PEND_REG0_OFFSET    0x10    // DMA IRQ 挂起寄存器0
#define DMA_IRQ_PEND_REG1_OFFSET    0x14    // DMA IRQ 挂起寄存器1

// DMA 安全寄存器和自动门控寄存器
#define DMA_SEC_REG_OFFSET          0x20    // DMA 安全寄存器
#define DMA_AUTO_GATE_REG_OFFSET    0x28    // DMA 自动门控寄存器
#define DMA_STA_REG_OFFSET          0x30    // DMA 状态寄存器

// DMA 通道专用寄存器 (N=0到11)
#define DMA_EN_REG_OFFSET(N)        (0x100 * (N) + 0x40 * 0x00)     // DMA 通道使能寄存器 (N=0到11)
#define DMA_PAU_REG_OFFSET(N)       (0x100 * (N) + 0x40 * 0x04)     // DMA 通道暂停寄存器 (N=0到11)
#define DMA_DESC_ADDR_REG_OFFSET(N) (0x100 * (N) + 0x40 * 0x08)     // DMA 通道开始地址寄存器 (N=0到11)
#define DMA_CFG_REG_OFFSET(N)       (0x100 * (N) + 0x40 * 0x0C)     // DMA 通道配置寄存器 (N=0到11)
#define DMA_CUR_SRC_REG_OFFSET(N)   (0x100 * (N) + 0x40 * 0x10)     // DMA 通道当前源地址寄存器 (N=0到11)
#define DMA_CUR_DEST_REG_OFFSET(N)  (0x100 * (N) + 0x40 * 0x14)     // DMA 通道当前目标地址寄存器 (N=0到11)
#define DMA_BCNT_LEFT_REG_OFFSET(N) (0x100 * (N) + 0x40 * 0x18)     // DMA 通道剩余字节计数器寄存器 (N=0到11)
#define DMA_PARA_REG_OFFSET(N)      (0x100 * (N) + 0x40 * 0x1C)     // DMA 通道参数寄存器 (N=0到11)
#define DMA_FDESC_ADDR_REG_OFFSET(N)(0x100 * (N) + 0x40 * 0x2C)     // DMA 前描述符地址寄存器 (N=0到11)
#define DMA_PKG_NUM_REG_OFFSET(N)   (0x100 * (N) + 0x40 * 0x30)     // DMA 数据包编号寄存器 (N=0到11)

// DMA_IRQ_EN_REG0 位域结构体定义，来自技术手册p193 4.11.4.1 DMA IRQ Enable Register0
typedef struct {
    uint32_t DMA0_HLAF_IRQ_EN   : 1;  // [0] DMA 0 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA0_PKG_IRQ_EN    : 1;  // [1] DMA 0 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA0_QUEUE_IRQ_EN  : 1;  // [2] DMA 0 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED3          : 1;  // [3] 保留未使用
    uint32_t DMA1_HLAF_IRQ_EN   : 1;  // [4] DMA 1 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA1_PKG_IRQ_EN    : 1;  // [5] DMA 1 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA1_QUEUE_IRQ_EN  : 1;  // [6] DMA 1 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED7          : 1;  // [7] 保留未使用
    uint32_t DMA2_HLAF_IRQ_EN   : 1;  // [8] DMA 2 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA2_PKG_IRQ_EN    : 1;  // [9] DMA 2 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA2_QUEUE_IRQ_EN  : 1;  // [10] DMA 2 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED11         : 1;  // [11] 保留未使用
    uint32_t DMA3_HLAF_IRQ_EN   : 1;  // [12] DMA 3 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA3_PKG_IRQ_EN    : 1;  // [13] DMA 3 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA3_QUEUE_IRQ_EN  : 1;  // [14] DMA 3 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED15         : 1;  // [15] 保留未使用
    uint32_t DMA4_HLAF_IRQ_EN   : 1;  // [16] DMA 4 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA4_PKG_IRQ_EN    : 1;  // [17] DMA 4 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA4_QUEUE_IRQ_EN  : 1;  // [18] DMA 4 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED19         : 1;  // [19] 保留未使用
    uint32_t DMA5_HLAF_IRQ_EN   : 1;  // [20] DMA 5 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA5_PKG_IRQ_EN    : 1;  // [21] DMA 5 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA5_QUEUE_IRQ_EN  : 1;  // [22] DMA 5 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED23         : 1;  // [23] 保留未使用
    uint32_t DMA6_HLAF_IRQ_EN   : 1;  // [24] DMA 6 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA6_PKG_IRQ_EN    : 1;  // [25] DMA 6 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA6_QUEUE_IRQ_EN  : 1;  // [26] DMA 6 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED27         : 1;  // [27] 保留未使用
    uint32_t DMA7_HLAF_IRQ_EN   : 1;  // [28] DMA 7 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA7_PKG_IRQ_EN    : 1;  // [29] DMA 7 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA7_QUEUE_IRQ_EN  : 1;  // [30] DMA 7 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED31         : 1;  // [31] 保留未使用
} DMA_IRQ_EN_REG0_t;

// DMA_IRQ_EN_REG1 位域结构体定义，来自技术手册p195 4.11.4.2 DMA IRQ Enable Register1
typedef struct {
    uint32_t DMA8_HLAF_IRQ_EN    : 1;  // [0] DMA 8 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA8_PKG_IRQ_EN     : 1;  // [1] DMA 8 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA8_QUEUE_IRQ_EN   : 1;  // [2] DMA 8 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED3           : 1;  // [3] 保留未使用
    uint32_t DMA9_HLAF_IRQ_EN    : 1;  // [4] DMA 9 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA9_PKG_IRQ_EN     : 1;  // [5] DMA 9 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA9_QUEUE_IRQ_EN   : 1;  // [6] DMA 9 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED7           : 1;  // [7] 保留未使用
    uint32_t DMA10_HLAF_IRQ_EN   : 1;  // [8] DMA 10 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA10_PKG_IRQ_EN    : 1;  // [9] DMA 10 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA10_QUEUE_IRQ_EN  : 1;  // [10] DMA 10 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED11          : 1;  // [11] 保留未使用
    uint32_t DMA11_HLAF_IRQ_EN   : 1;  // [12] DMA 11 半包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA11_PKG_IRQ_EN    : 1;  // [13] DMA 11 完整包传输中断使能位，0: 禁用，1: 使能
    uint32_t DMA11_QUEUE_IRQ_EN  : 1;  // [14] DMA 11 队列结束传输中断使能位，0: 禁用，1: 使能
    uint32_t RESERVED15_31       : 17; // [15:31] 保留未使用
} DMA_IRQ_EN_REG1_t;

// DMA_IRQ_PEND_REG0 位域结构体定义，来自技术手册p196 4.11.4.3 DMA IRQ Pending Status Register0
typedef struct {
    uint32_t DMA0_HLAF_IRQ_PEND   : 1;  // [0] DMA 0 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA0_PKG_IRQ_PEND    : 1;  // [1] DMA 0 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA0_QUEUE_IRQ_PEND  : 1;  // [2] DMA 0 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED3            : 1;  // [3] 保留未使用
    uint32_t DMA1_HLAF_IRQ_PEND   : 1;  // [4] DMA 1 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA1_PKG_IRQ_PEND    : 1;  // [5] DMA 1 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA1_QUEUE_IRQ_PEND  : 1;  // [6] DMA 1 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED7            : 1;  // [7] 保留未使用
    uint32_t DMA2_HLAF_IRQ_PEND   : 1;  // [8] DMA 2 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA2_PKG_IRQ_PEND    : 1;  // [9] DMA 2 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA2_QUEUE_IRQ_PEND  : 1;  // [10] DMA 2 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED11           : 1;  // [11] 保留未使用
    uint32_t DMA3_HLAF_IRQ_PEND   : 1;  // [12] DMA 3 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA3_PKG_IRQ_PEND    : 1;  // [13] DMA 3 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA3_QUEUE_IRQ_PEND  : 1;  // [14] DMA 3 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED15           : 1;  // [15] 保留未使用
    uint32_t DMA4_HLAF_IRQ_PEND   : 1;  // [16] DMA 4 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA4_PKG_IRQ_PEND    : 1;  // [17] DMA 4 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA4_QUEUE_IRQ_PEND  : 1;  // [18] DMA 4 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED19           : 1;  // [19] 保留未使用
    uint32_t DMA5_HLAF_IRQ_PEND   : 1;  // [20] DMA 5 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA5_PKG_IRQ_PEND    : 1;  // [21] DMA 5 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA5_QUEUE_IRQ_PEND  : 1;  // [22] DMA 5 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED23           : 1;  // [23] 保留未使用
    uint32_t DMA6_HLAF_IRQ_PEND   : 1;  // [24] DMA 6 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA6_PKG_IRQ_PEND    : 1;  // [25] DMA 6 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA6_QUEUE_IRQ_PEND  : 1;  // [26] DMA 6 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED27           : 1;  // [27] 保留未使用
    uint32_t DMA7_HLAF_IRQ_PEND   : 1;  // [28] DMA 7 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA7_PKG_IRQ_PEND    : 1;  // [29] DMA 7 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA7_QUEUE_IRQ_PEND  : 1;  // [30] DMA 7 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED31           : 1;  // [31] 保留未使用
} DMA_IRQ_PEND_REG0_t;

// DMA_IRQ_PEND_REG1 位域结构体定义，来自技术手册p198 4.11.4.4 DMA IRQ Pending Status Register1
typedef struct {
    uint32_t DMA8_HLAF_IRQ_PEND    : 1;  // [0] DMA 8 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA8_PKG_IRQ_PEND     : 1;  // [1] DMA 8 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA8_QUEUE_IRQ_PEND   : 1;  // [2] DMA 8 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED3             : 1;  // [3] 保留未使用
    uint32_t DMA9_HLAF_IRQ_PEND    : 1;  // [4] DMA 9 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA9_PKG_IRQ_PEND     : 1;  // [5] DMA 9 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA9_QUEUE_IRQ_PEND   : 1;  // [6] DMA 9 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED7             : 1;  // [7] 保留未使用
    uint32_t DMA10_HLAF_IRQ_PEND   : 1;  // [8] DMA 10 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA10_PKG_IRQ_PEND    : 1;  // [9] DMA 10 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA10_QUEUE_IRQ_PEND  : 1;  // [10] DMA 10 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED11            : 1;  // [11] 保留未使用
    uint32_t DMA11_HLAF_IRQ_PEND   : 1;  // [12] DMA 11 半包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA11_PKG_IRQ_PEND    : 1;  // [13] DMA 11 完整包传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t DMA11_QUEUE_IRQ_PEND  : 1;  // [14] DMA 11 队列结束传输中断挂起，0: 无效，1: 挂起（设置为1时清除挂起位）
    uint32_t RESERVED15_31         : 17; // [15:31] 保留未使用
} DMA_IRQ_PEND_REG1_t;

// DMA_SECURE_REG 位域结构体定义，来自技术手册p199 4.11.4.5 DMA Security Register
typedef struct {
    uint32_t DMA0_SEC    : 1;  // [0] DMA 0 通道安全位，0: 安全，1: 非安全
    uint32_t DMA1_SEC    : 1;  // [1] DMA 1 通道安全位，0: 安全，1: 非安全
    uint32_t DMA2_SEC    : 1;  // [2] DMA 2 通道安全位，0: 安全，1: 非安全
    uint32_t DMA3_SEC    : 1;  // [3] DMA 3 通道安全位，0: 安全，1: 非安全
    uint32_t DMA4_SEC    : 1;  // [4] DMA 4 通道安全位，0: 安全，1: 非安全
    uint32_t DMA5_SEC    : 1;  // [5] DMA 5 通道安全位，0: 安全，1: 非安全
    uint32_t DMA6_SEC    : 1;  // [6] DMA 6 通道安全位，0: 安全，1: 非安全
    uint32_t DMA7_SEC    : 1;  // [7] DMA 7 通道安全位，0: 安全，1: 非安全
    uint32_t DMA8_SEC    : 1;  // [8] DMA 8 通道安全位，0: 安全，1: 非安全
    uint32_t DMA9_SEC    : 1;  // [9] DMA 9 通道安全位，0: 安全，1: 非安全
    uint32_t DMA10_SEC   : 1;  // [10] DMA 10 通道安全位，0: 安全，1: 非安全
    uint32_t DMA11_SEC   : 1;  // [11] DMA 11 通道安全位，0: 安全，1: 非安全
    uint32_t RESERVED12_31 : 20;  // [12:31] 保留未使用
} DMA_SECURE_REG_t;

// DMA_AUTO_GATE_REG 位域结构体定义，来自技术手册p200 4.11.4.6 DMA Auto Gating Register
typedef struct {
    uint32_t DMA_CHAN_CIRCUIT    : 1;  // [0] DMA 通道电路自动门控位，0: 自动门控使能，1: 自动门控禁用
    uint32_t DMA_COMMON_CIRCUIT  : 1;  // [1] DMA 通用电路自动门控位，0: 自动门控使能，1: 自动门控禁用
    uint32_t DMA_MCLK_CIRCUIT    : 1;  // [2] DMA MCLK 接口电路自动门控位，0: 自动门控使能，1: 自动门控禁用
    uint32_t RESERVED3_31        : 29; // [3:31] 保留未使用
} DMA_AUTO_GATE_REG_t;

// DMA_STA_REG 位域结构体定义，来自技术手册p201 4.11.4.7 DMA Status Register
typedef struct {
    uint32_t DMA0_STATUS    : 1;    // [0] DMA 通道 0 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA1_STATUS    : 1;    // [1] DMA 通道 1 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA2_STATUS    : 1;    // [2] DMA 通道 2 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA3_STATUS    : 1;    // [3] DMA 通道 3 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA4_STATUS    : 1;    // [4] DMA 通道 4 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA5_STATUS    : 1;    // [5] DMA 通道 5 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA6_STATUS    : 1;    // [6] DMA 通道 6 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA7_STATUS    : 1;    // [7] DMA 通道 7 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA8_STATUS    : 1;    // [8] DMA 通道 8 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA9_STATUS    : 1;    // [9] DMA 通道 9 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA10_STATUS   : 1;    // [10] DMA 通道 10 状态，0: 空闲，1: 忙碌。只读。
    uint32_t DMA11_STATUS   : 1;    // [11] DMA 通道 11 状态，0: 空闲，1: 忙碌。只读。
    uint32_t RESERVED12_29  : 18;   // [12:29] 保留未使用
    uint32_t MBUS_FIFO_STATUS : 1;  // [30] MBUS FIFO 状态，0: 空，1: 非空。只读。
    uint32_t RESERVED31     : 1;    // [31] 保留未使用
} DMA_STA_REG_t;

// DMA_EN_REG 位域结构体定义，来自技术手册p202 4.11.4.8 DMA Channel Enable Register
typedef struct {
    uint32_t DMA_EN      : 1;  // [0] DMA 通道使能，0: 禁用，1: 使能
    uint32_t RESERVED1_31: 31; // [1:31] 保留未使用
} DMA_EN_REG_t;

// DMA_PAU_REG 位域结构体定义，来自技术手册p203 4.11.4.9 DMA Channel Pause Register
typedef struct {
    uint32_t DMA_PAUSE     : 1;  // [0] 暂停 DMA 通道传输，0: 恢复传输，1: 暂停传输
    uint32_t RESERVED1_31  : 31; // [1:31] 保留未使用
} DMA_PAU_REG_t;

// DMA_DESC_ADDR_REG 位域结构体定义，来自技术手册p203 4.11.4.10 DMA Channel Descriptor Address Register
typedef struct {
    uint32_t DMA_DESC_ADDR : 32; // [0:31] DMA 通道描述符地址，必须是字对齐的
} DMA_DESC_ADDR_REG_t;

// DMA_CFG_REG 位域结构体定义，来自技术手册p203 4.11.4.11 DMA Channel Configuration Register
typedef struct {
    uint32_t DMA_SRC_DRQ_TYPE   : 5;  // [0:4] DMA 源端 DRQ 类型，只读
    uint32_t DMA_SRC_ADDR_MODE  : 1;  // [5] DMA 源端地址模式，0: 线性模式, 1: IO模式，只读
    uint32_t DMA_SRC_BST_LEN    : 2;  // [6:7] DMA 源端突发长度，00: 1, 01: 4, 10: 8, 11: 16，只读
    uint32_t RESERVED8          : 1;  // [8] 保留位
    uint32_t DMA_SRC_DATA_WIDTH : 2;  // [9:10] DMA 源端数据宽度，00: 8位, 01: 16位, 10: 32位, 11: 64位，只读
    uint32_t RESERVED11_15      : 5;  // [11:15] 保留位
    uint32_t DMA_DEST_DRQ_TYPE  : 5;  // [16:20] DMA 目的端 DRQ 类型，只读
    uint32_t DMA_DEST_ADDR_MODE : 1;  // [21] DMA 目的端地址模式，0: 线性模式, 1: IO模式，只读
    uint32_t DMA_DEST_BST_LEN   : 2;  // [22:23] DMA 目的端突发长度，00: 1, 01: 4, 10: 8, 11: 16，只读
    uint32_t RESERVED24         : 1;  // [24] 保留位
    uint32_t DMA_DEST_DATA_WIDTH: 2;  // [25:26] DMA 目的端数据宽度，00: 8位, 01: 16位, 10: 32位, 11: 64位，只读
    uint32_t RESERVED27_31      : 5;  // [27:31] 保留位
} DMA_CFG_REG_t;

// DMA_CUR_SRC_REG 位域结构体定义，来自技术手册p203 4.11.4.12 DMA Channel Current Source Address Register
typedef struct {
    uint32_t DMA_CUR_SRC_ADDR : 32;  // [0:31] DMA 通道当前源地址，只读
} DMA_CUR_SRC_REG_t;

// DMA_CUR_DEST_REG 位域结构体定义，来自技术手册p203 4.11.4.13 DMA Channel Current Destination Address Register
typedef struct {
    uint32_t DMA_CUR_DEST_ADDR : 32;  // [0:31] DMA 通道当前目的地址，只读
} DMA_CUR_DEST_REG_t;

// DMA_BCNT_LEFT_REG 位域结构体定义，来自技术手册p204 4.11.4.14 DMA Channel Byte Counter Left Register
typedef struct {
    uint32_t DMA_BCNT_LEFT : 25;  // [0:24] DMA 通道剩余字节计数，只读
    uint32_t RESERVED25_31 : 7;   // [25:31] 保留位
} DMA_BCNT_LEFT_REG_t;

// DMA_PARA_REG 位域结构体定义，来自技术手册p204 4.11.4.15 DMA Channel Parameter Register
typedef struct {
    uint32_t WAIT_CYC     : 8;   // [0:7] 等待时钟周期数，只读
    uint32_t RESERVED8_31 : 24;  // [8:31] 保留位
} DMA_PARA_REG_t;

// DMA_FDESC_ADDR_REG 位域结构体定义，来自技术手册p204 4.11.4.16 DMA Former Descriptor Address Register
typedef struct {
    uint32_t DMA_FDESC_ADDR : 32;  // [0:31] 前描述符地址，用于存储 DMA 通道描述符地址寄存器的前一个值，只读
} DMA_FDESC_ADDR_REG_t;

// DMA_PKG_NUM_REG 位域结构体定义，来自技术手册p205 4.11.4.17 DMA Package Number Register
typedef struct {
    uint32_t DMA_PKG_NUM : 32;  // [0:31] 记录在一次传输中已经完成的数据包数量，只读
} DMA_PKG_NUM_REG_t;


// 以下是本项目定义的结构体

// 硬件DMA控制器所需的描述符结构体定义，该结构体实例的地址写入到 DMA_DESC_ADDR_REG_OFFSET(N) 中
typedef struct hc_sun6i_dma_descriptor {
    DMA_CFG_REG_t cfg;                  // DMA配置项
    DMA_CUR_SRC_REG_t src;              // 源地址
    DMA_CUR_DEST_REG_t dst;             // 目的地址
    uint32_t len;                       // 数据长度
    DMA_PARA_REG_t para;                // 传输的附加参数
    dma_addr_t  p_next_dma_descriptor;  // 下一个描述符的物理地址（如果没有下一个，就要置为DMA_DESCRIPTOR_END_ADDRESS，即0xFFFFF800）

    // 解释说明
    // dma_addr_t 是在 Linux 内核中用来表示 DMA（直接内存访问）操作时物理地址的类型（32位机器是uint32_t，64位机器是uint64_t）。

    // DMA控制器读到第六个参数，也就是p_next_dma_descriptor就会直接跳转到下一个描述符
    // v_next_dma_descriptor是用来给软件控制逻辑判断用的
    // DMA控制器是硬件，不受CPU的MMU影响，所以只能是物理地址
    // 但是如果是在CPU上运行，受MMU控制，不能直接访问物理地址，还是需要虚拟地址的

    struct hc_sun6i_dma_descriptor *v_next_dma_descriptor;  // 指向下一个描述符的虚拟地址

} DMA_HardWare_Descriptor;

// 驱动使用的DMA任务描述符，实际上就是简单包装一下DMA_HardWare_Descriptor，获取相关信息的时候方便一些，并继承Linux内核DMA驱动框架的虚拟通道描述符
typedef struct hc_dma_task_descriptor {
    struct virt_dma_desc    vd;             // Linux内核的虚拟DMA框架的DMA任务描述符，继承这个，就可以让Linux内核自动管理这个描述符
    dma_addr_t * physical_addr;             // 描述符的物理地址
    DMA_HardWare_Descriptor * virtual_addr; // 描述符的虚拟地址
} DMA_TASK_Descriptor;

// 前向声明虚拟通道结构体，后面再定义
typedef struct hc_sun6i_dma_virtual_channel_info DMA_Virtual_Channel_Info;

// 该结构体记录一个DMA物理通道的所有信息
typedef struct hc_sun6i_dma_physical_channel_info {
    uint32_t            index;          // 物理通道编号
    void __iomem *      base_addr;      // 物理通道的基址

    DMA_TASK_Descriptor *todo;          // To Do，待办事项，就是没完成的任务
    DMA_TASK_Descriptor *done;          // done，都完成时了，自然就是已经完成的任务

    DMA_Virtual_Channel_Info *  vchan;  // 该物理通道关联的虚拟通道
} DMA_Physical_Channel_Info;

// 该结构体记录一个DMA虚拟通道的所有信息
struct hc_sun6i_dma_virtual_channel_info {
    struct virt_dma_chan    vc;         // Linux内核DMA驱动框架提供的虚拟DMA通道结构体
    DMA_Physical_Channel_Info * pchan;  // 当前绑定的物理通道（如果当前无任务应为NULL）

    struct dma_slave_config cfg;        // 该虚拟通道的从设备配置
    uint32_t			    port;       // 该虚拟通道对应的DRQ端口
    uint32_t			    irq_type;   // 中断类型
    bool			        cyclic;     // 该虚拟通道是否为循环DMA模式

    struct list_head        task_queue_head;    // 该虚拟通道的任务队列
    DMA_TASK_Descriptor *   task;               // 该虚拟通道当前正在处理的任务
};

// 该结构体记录适配型号的不同DMA的各项参数
typedef struct hc_dma_config {
    uint32_t max_physical_channels;     // 最大物理通道数
    uint32_t max_virtual_channels;      // 最大虚拟通道数
    uint32_t max_requests;              // 最大的DRQ端口ID
} DMA_Config;

// 该结构体记录这个DMA控制器的所有信息
typedef struct hc_dma_dev_info {
    struct dma_device   slave;              // DMA设备的抽象结构
    void __iomem *      base_addr;          // DMA控制器的寄存器基地址
    struct clk *        clk;                // 时钟控制结构
    int                 irq;                // DMA控制器的中断号
    spinlock_t          lock;               // 自旋锁，用于保护共享资源
    struct reset_control *  rstc;           // 复位控制结构
    struct tasklet_struct   task;           // 任务结构
    atomic_t            tasklet_shutdown;   // 原子变量，用于标记tasklet的运行状态
    struct list_head    pending;            // 挂起的DMA任务双向链表
    struct dma_pool *   pool;               // DMA内存池

    DMA_Physical_Channel_Info * pchans;     // 物理通道数组
    DMA_Virtual_Channel_Info *  vchans;     // 虚拟通道数组

    const DMA_Config *  cfg;                // DMA控制器的配置信息，定义了通道数量、请求数量等
} DMA_DEV_Info;

// 以下是本项目需要完成的函数

// 以下是函数声明
static int hc_dma_probe(struct platform_device *pdev);
static int hc_dma_remove(struct platform_device *pdev);
static void hc_dma_tasklet(unsigned long data);
static irqreturn_t hc_dma_interrupt(int irq, void *dev_id);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src, size_t len, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_slave_sg(struct dma_chan *chan, struct scatterlist *sgl, unsigned int sg_len, enum dma_transfer_direction dir, unsigned long flags, void *context);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_cyclic(struct dma_chan *chan, dma_addr_t buf_addr, size_t buf_len, size_t period_len, enum dma_transfer_direction dir, unsigned long flags);
static int hc_dma_config(struct dma_chan *chan, struct dma_slave_config *config);
static int hc_dma_pause(struct dma_chan *chan);
static int hc_dma_resume(struct dma_chan *chan);
static int hc_dma_terminate_all(struct dma_chan *chan);
static enum dma_status hc_dma_tx_status(struct dma_chan *chan, dma_cookie_t cookie, struct dma_tx_state *state);
static void hc_dma_issue_pending(struct dma_chan *chan);
static int hc_dma_alloc_chan_resources(struct dma_chan *chan);
static void hc_dma_free_chan_resources(struct dma_chan *chan);
static int hc_dma_start_transfer(struct dma_chan *chan);
static void hc_dma_stop_transfer(struct dma_chan *chan);

// 以下是平台驱动匹配信息

// 目前只支持Allwinner H3平台，所以也只有这个了
DMA_Config sun8i_h3_dma_cfg = {
        .max_physical_channels = 12,    // 技术手册里面说了物理通道数就12
        .max_virtual_channels = 30,     // Linux内核里面sun6i的DMA驱动用的数值是34，不懂怎么回事，可能是测试出来的经验数据，我这里取个整数30
        .max_requests = 27              // 最大的DRQ端口ID，因为最后一个有实际意义的只到26，后面都没有意义，所以定为27
};

// 匹配设备树中的`compatible`字段，通过这个，找到对应的结点
static struct of_device_id hc_dma_of_match[] = {
        { .compatible = "allwinner,sun8i-h3-dma", .data = &sun8i_h3_dma_cfg },
        { },
};
MODULE_DEVICE_TABLE(of, hc_dma_of_match);

// 平台驱动结构体，通过of_match_table来匹配到对应的结点，进而载入平台设备
static struct platform_driver hc_dma_driver = {
        .probe = hc_dma_probe,
        .remove = hc_dma_remove,
        .driver = {
                .name = DEVICE_NAME,
                .of_match_table = hc_dma_of_match,
                .owner = THIS_MODULE,
        },
};
module_platform_driver(hc_dma_driver);

// 以下是函数实现

/*
 * DMA探测函数
 * 功能: 当平台设备被探测到时，初始化DMA控制器。
 */
static int hc_dma_probe(struct platform_device *pdev) {
    // 1. 从设备树中获取DMA控制器的资源信息（寄存器基地址、中断号、时钟资源等）。
    // 2. 启用并初始化DMA控制器的时钟。
    // 3. 申请DMA控制器的中断号，并注册中断处理程序。
    // 4. 初始化DMA物理通道和虚拟通道的结构体，分配资源。
    // 5. 初始化任务队列、描述符池、任务调度器等内核资源。
    // 6. 注册DMA设备到内核DMA框架，供其他驱动使用。
    // 7. 返回成功或者失败的状态。
}

/*
 * DMA移除函数
 * 功能: 当平台设备被移除时，释放资源并进行清理。
 */
static int hc_dma_remove(struct platform_device *pdev) {
    // 1. 停止所有进行中的DMA操作，确保DMA任务全部结束。
    // 2. 释放DMA中断资源，注销中断处理程序。
    // 3. 释放DMA时钟资源，停用时钟。
    // 4. 释放物理通道和虚拟通道占用的资源。
    // 5. 清理DMA控制器的任务队列、描述符池等内核资源。
    // 6. 返回成功或者失败的状态。
}

/*
 * DMA Tasklet函数
 * 功能: 使用tasklet机制处理DMA操作中的任务调度和管理。
 */
static void hc_dma_tasklet(unsigned long data) {
    // 1. 获取tasklet相关的DMA控制器结构体信息。
    // 2. 遍历任务队列，检查是否有待处理的DMA任务。
    // 3. 根据DMA任务描述符，配置DMA控制器寄存器。
    // 4. 启动DMA传输。
    // 5. 完成当前任务后，检查是否有下一个任务，若有则继续调度。
}

/*
 * DMA中断处理程序
 * 功能: 处理DMA控制器产生的中断。
 */
static irqreturn_t hc_dma_interrupt(int irq, void *dev_id) {
    // 1. 检查DMA控制器的中断状态寄存器，确认触发的中断类型。
    // 2. 对于DMA传输完成或错误中断，清除中断标志。
    // 3. 如果是传输完成中断，唤醒tasklet处理下一个任务。
    // 4. 返回中断处理的状态（IRQ_HANDLED 或者 IRQ_NONE）。
}

/*
 * DMA描述符准备函数
 * 功能: 为不同类型的DMA操作准备DMA描述符。
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memcpy(
        struct dma_chan *chan, dma_addr_t dest, dma_addr_t src, size_t len, unsigned long flags) {
    // 1. 检查传入的源地址和目的地址是否合法。
    // 2. 在描述符池中分配一个新的DMA描述符。
    // 3. 设置DMA描述符的源地址、目的地址、传输长度等字段。
    // 4. 将描述符加入DMA任务队列。
    // 5. 返回准备好的DMA事务描述符。
}

static struct dma_async_tx_descriptor *hc_dma_prep_slave_sg(
        struct dma_chan *chan, struct scatterlist *sgl, unsigned int sg_len,
        enum dma_transfer_direction dir, unsigned long flags, void *context) {
    // 1. 遍历scatter-gather列表，检查每个段的地址和长度是否合法。
    // 2. 分配并设置对应的DMA描述符，填入scatter-gather的地址和长度信息。
    // 3. 根据DMA传输方向（内存到设备或设备到内存），设置描述符的源和目的地址。
    // 4. 将所有描述符链接在一起，形成描述符链。
    // 5. 将描述符链加入DMA任务队列，并返回事务描述符。
}

static struct dma_async_tx_descriptor *hc_dma_prep_dma_cyclic(
        struct dma_chan *chan, dma_addr_t buf_addr, size_t buf_len, size_t period_len,
        enum dma_transfer_direction dir, unsigned long flags) {
    // 1. 检查传入的缓冲区地址和长度是否合法。
    // 2. 为周期性DMA操作分配多个描述符，每个描述符处理一个周期长度的数据。
    // 3. 将描述符链接成环状，形成循环DMA操作。
    // 4. 设置DMA传输方向（内存到设备或设备到内存），配置描述符中的地址。
    // 5. 将描述符链加入任务队列，并返回事务描述符。
}

/*
 * DMA通道配置函数
 * 功能: 配置DMA通道的相关设置（地址宽度、突发长度等）。
 */
static int hc_dma_config(struct dma_chan *chan, struct dma_slave_config *config) {
    // 1. 检查传入的配置参数是否合法。
    // 2. 配置DMA通道的源地址宽度、目的地址宽度、突发长度等参数。
    // 3. 更新DMA通道配置寄存器。
    // 4. 将新的配置应用到对应的DMA通道。
    // 5. 返回配置结果（成功或失败）。
}

/*
 * DMA通道控制函数
 * 功能: 控制DMA通道的暂停、恢复和终止操作。
 */
static int hc_dma_pause(struct dma_chan *chan) {
    // 1. 获取DMA通道的状态，检查是否正在传输数据。
    // 2. 如果通道正在传输，暂停DMA控制器对该通道的访问。
    // 3. 更新通道状态为暂停。
    // 4. 返回暂停操作的结果（成功或失败）。
}

static int hc_dma_resume(struct dma_chan *chan) {
    // 1. 检查DMA通道当前是否处于暂停状态。
    // 2. 如果通道处于暂停状态，恢复DMA控制器对该通道的访问。
    // 3. 继续未完成的DMA传输操作。
    // 4. 返回恢复操作的结果（成功或失败）。
}

static int hc_dma_terminate_all(struct dma_chan *chan) {
    // 1. 停止DMA控制器对该通道的所有访问，清除挂起的任务。
    // 2. 释放通道上的所有DMA描述符和任务资源。
    // 3. 将通道状态更新为终止状态。
    // 4. 返回终止操作的结果（成功或失败）。
}

/*
 * DMA传输状态函数
 * 功能: 获取DMA传输的状态。
 */
static enum dma_status hc_dma_tx_status(
        struct dma_chan *chan, dma_cookie_t cookie, struct dma_tx_state *state) {
    // 1. 检查DMA传输任务的cookie，判断该任务是否仍在进行中。
    // 2. 如果任务已经完成，更新传输状态为DMA_COMPLETE。
    // 3. 如果任务尚未完成，检查传输进度，更新state结构体。
    // 4. 返回当前传输任务的状态（例如：DMA_IN_PROGRESS、DMA_ERROR、DMA_COMPLETE）。
}

/*
 * DMA提交待处理函数
 * 功能: 提交待处理的DMA事务。
 */
static void hc_dma_issue_pending(struct dma_chan *chan) {
    // 1. 检查DMA任务队列是否有待处理的任务。
    // 2. 将任务队列中的描述符提交给DMA控制器。
    // 3. 如果DMA控制器空闲，立即启动传输。
    // 4. 如果DMA控制器忙碌，等待当前任务完成后处理下一个任务。
}

/*
 * DMA通道分配函数
 * 功能: 为每个虚拟或物理DMA通道分配资源。
 */
static int hc_dma_alloc_chan_resources(struct dma_chan *chan) {
    // 1. 分配通道相关的资源，包括描述符池、任务队列等。
    // 2. 初始化通道的控制结构体和相关寄存器。
    // 3. 将通道注册到内核DMA框架中。
    // 4. 返回资源分配的结果（成功或失败）。
}

/*
 * 释放通道资源函数
 * 功能: 释放与DMA通道相关的资源。
 */
static void hc_dma_free_chan_resources(struct dma_chan *chan) {
    // 1. 停止该通道上的所有DMA任务。
    // 2. 释放描述符池和任务队列中的资源。
    // 3. 更新通道状态为未使用。
    // 4. 将通道从内核DMA框架中注销。
}

/*
 * DMA传输启动函数
 * 功能: 启动DMA传输操作。
 */
static int hc_dma_start_transfer(struct dma_chan *chan) {
    // 1. 检查通道是否有待处理的DMA任务。
    // 2. 配置DMA控制器的寄存器，开始执行传输。
    // 3. 启动DMA控制器对该通道的访问。
    // 4. 返回传输启动的结果（成功或失败）。
}

/*
 * DMA传输停止函数
 * 功能: 停止DMA传输操作。
 */
static void hc_dma_stop_transfer(struct dma_chan *chan) {
    // 1. 停止DMA控制器对该通道的访问，终止传输。
    // 2. 清除通道上的传输状态，释放相关资源。
    // 3. 将通道状态更新为停止。
}

// 以下是驱动作者信息和描述、版本、许可证信息

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng, <huangcheng20000702@gmail.com> ");
MODULE_DESCRIPTION("A dma controller driver for OrangePi One");
MODULE_VERSION("1.0");
