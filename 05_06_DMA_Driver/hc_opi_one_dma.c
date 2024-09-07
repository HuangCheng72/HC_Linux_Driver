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

// 新增头文件
#include <linux/io.h>

// 新增函数声明

static void hc_free_dma_task_descriptor(struct virt_dma_desc *vd);
static struct dma_chan *hc_dma_of_xlate(struct of_phandle_args *dma_spec, struct of_dma *ofdma);

/*
 * DMA探测函数
 * 功能: 当平台设备被探测到时，初始化DMA控制器。
 */
static int hc_dma_probe(struct platform_device *pdev) {
    struct resource *res;               // 资源
    DMA_DEV_Info *dma_dev;              // DMA设备信息结构体
    const struct of_device_id *match;   // 匹配表信息
    int ret, i;                         // 返回结果和计数器

    // 给DMA设备信息结构体分配内存
    dma_dev = devm_kzalloc(&pdev->dev, sizeof(*dma_dev), GFP_KERNEL);
    if (!dma_dev) {
        // 分配失败返回
        return -ENOMEM;
    }

    // 获取匹配表里面的配置数据（也就是物理通道数、虚拟通道数、最大请求ID）
    match = of_match_device(hc_dma_of_match, &pdev->dev);
    if (!match) {
        return -EINVAL;
    }
    dma_dev->cfg = match->data;

    // 从设备树获取资源
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    // 直接从资源里面读出物理地址和映射大小，然后建立映射关系
    dma_dev->base_addr = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(dma_dev->base_addr)) {
        return PTR_ERR(dma_dev->base_addr);
    }

    // 从设备树获取IRQ中断号
    dma_dev->irq = platform_get_irq(pdev, 0);
    if (dma_dev->irq < 0) {
        dev_err(&pdev->dev, "Failed to get IRQ\n");
        return dma_dev->irq;
    }
    // 向内核申请对应的IRQ中断，参数是设备、中断号、中断处理函数、设备名、设备数据
    ret = devm_request_irq(&pdev->dev, dma_dev->irq, hc_dma_interrupt, 0, pdev->name, dma_dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request IRQ\n");
        return ret;
    }

    // 从设备中获取DMA控制器需要的时钟控制资源
    dma_dev->clk = devm_clk_get(&pdev->dev, NULL);
    if (IS_ERR(dma_dev->clk)) {
        dev_err(&pdev->dev, "Failed to get clock\n");
        return PTR_ERR(dma_dev->clk);
    }

    // 从设备中获取DMA控制器需要的复位控制资源
    dma_dev->rstc = devm_reset_control_get(&pdev->dev, NULL);
    if (IS_ERR(dma_dev->rstc)) {
        dev_err(&pdev->dev, "Failed to get reset controller\n");
        return PTR_ERR(dma_dev->rstc);
    }

    // 创建管理DMA描述符的内存池，用于管理硬件DMA控制器所需的描述符结构体DMA_HardWare_Descriptor
    // 参数为：设备名，设备结构体，描述符大小，对齐要求（这里要求4字节），内存分配粒度（0是默认）
    dma_dev->pool = dmam_pool_create(dev_name(&pdev->dev), &pdev->dev, sizeof(DMA_HardWare_Descriptor), 4, 0);
    if (!dma_dev->pool) {
        dev_err(&pdev->dev, "Failed to create DMA descriptor pool\n");
        return -ENOMEM;
    }

    // 初始化自旋锁
    spin_lock_init(&dma_dev->lock);
    // 初始化当前DMA设备挂起排队的任务的双向链表头结点
    INIT_LIST_HEAD(&dma_dev->pending);
    // 初始化当前DMA设备具有的通道链表头结点
    INIT_LIST_HEAD(&dma_dev->slave.channels);

    // 设置该DMA设备的功能，指定支持的功能

    // DMA_PRIVATE: 表示该设备只能被某些特定的设备使用，通常指的是专有的、私有的DMA通道，
    // 这里的私有指的是不会和其他外设共享通道，这种类型的DMA不被其他设备抢占或复用，用于保证特定设备的DMA任务。
    dma_cap_set(DMA_PRIVATE, dma_dev->slave.cap_mask);

    // DMA_MEMCPY: 该DMA设备支持内存到内存的传输（Memory Copy）。这是最常见的DMA操作之一，
    // 用于在不同内存区域之间快速移动数据。
    dma_cap_set(DMA_MEMCPY, dma_dev->slave.cap_mask);

    // DMA_SLAVE: 该设备支持从设备到内存，或者内存到设备的传输。这通常用于外设传输数据的场景，
    // 例如从UART、I2C等外设进行数据传输时，通过DMA加速数据读写。
    // DMA_SLAVE 是指 DMA 控制器将外设视为“主设备”，它在主设备与内存之间传输数据。
    dma_cap_set(DMA_SLAVE, dma_dev->slave.cap_mask);

    // DMA_CYCLIC: 支持循环传输模式，这种模式常用于音频、视频等实时数据流的传输，数据可以在缓冲区中
    // 循环传输，而无需在每次传输完成后重新设置地址或重新配置DMA。
    dma_cap_set(DMA_CYCLIC, dma_dev->slave.cap_mask);


    // 指定该DMA设备的各个功能的函数指针（slave这个结构体成员的类型来自于Linux源代码目录下的include/linux/dmaengine.h，第712行开始）
    dma_dev->slave.device_alloc_chan_resources = hc_dma_alloc_chan_resources;     // 分配DMA通道资源
    dma_dev->slave.device_free_chan_resources  = hc_dma_free_chan_resources;      // 释放DMA通道资源
    dma_dev->slave.device_prep_dma_memcpy      = hc_dma_prep_dma_memcpy;          // 准备一个内存拷贝操作

    // 缺失device_prep_dma_xor，该函数用于内存数据块的异或操作（常用于 RAID 校验）。
    // 主要用于 RAID 系统中，通过异或多个数据块生成校验数据块。

    // 缺失device_prep_dma_xor_val，该函数用于检查多个数据块之间的 XOR 和验证。
    // 用于验证多个数据块之间的异或校验结果，确保数据完整性，通常在 RAID 校验中使用。

    // 缺失device_prep_dma_pq，该函数用于准备 PQ 操作，常用于高级 RAID 校验（例如 RAID-6）。
    // PQ 操作涉及两个校验数据块，通常用于 RAID-6 级别的数据校验。

    // 缺失device_prep_dma_pq_val，该函数用于验证 PQ 校验和。
    // 用于 RAID-6 校验，验证生成的 PQ 校验数据是否正确。

    // 缺失device_prep_dma_memset，该函数用于准备 DMA 内存清零操作（如 memset）。
    // 通过 DMA 进行内存的填充操作，例如将内存块初始化为一个特定的值（如清零）。

    // 缺失device_prep_dma_memset_sg，该函数用于准备 scatter-gather 形式的内存填充操作。
    // 类似于 memset，但操作的是一个 scatter-gather 列表，用于分散的内存块进行填充。

    // 缺失device_prep_dma_interrupt，该函数用于准备传输完成时触发的中断。
    // 当 DMA 操作完成后，通过中断通知处理器，通常用于实时通知系统传输完成。

    // 缺失device_prep_dma_sg，该函数用于准备散列-聚集（scatter-gather）的传输操作。
    // 用于对分散的内存块进行传输，可以将多个不连续的内存块聚合到一起，或将数据分散到不同块中。

    dma_dev->slave.device_prep_slave_sg        = hc_dma_prep_slave_sg;            // 准备一个从设备的scatter-gather传输
    dma_dev->slave.device_prep_dma_cyclic      = hc_dma_prep_dma_cyclic;          // 准备一个循环DMA操作

    // 缺失device_prep_interleaved_dma，该函数用于准备交错传输，适合复杂数据结构。
    // 用于处理交错数据传输的场景，例如不同步的数据传输，或多个源/目的地址的复杂传输模式。

    // 缺失device_prep_dma_imm_data，该函数用于准备 DMA 的立即数据传输。
    // 将 8 字节的立即数据传输到目的地址，通常用于传输较小的数据块而不需要大型内存缓冲区。

    dma_dev->slave.device_config               = hc_dma_config;                   // 配置DMA通道
    dma_dev->slave.device_pause                = hc_dma_pause;                    // 暂停DMA传输
    dma_dev->slave.device_resume               = hc_dma_resume;                   // 恢复暂停的DMA传输
    dma_dev->slave.device_terminate_all        = hc_dma_terminate_all;            // 终止所有正在进行的DMA传输

    // 缺失device_synchronize，该函数用于同步DMA的终止操作。
    // 确保 DMA 通道在终止操作时所有传输任务安全结束，避免数据丢失或资源争用。

    dma_dev->slave.device_tx_status            = hc_dma_tx_status;                // 获取DMA传输状态
    dma_dev->slave.device_issue_pending        = hc_dma_issue_pending;            // 推送挂起的DMA传输任务

    // 指定DMA各项属性

    // 设置内存拷贝操作的对齐方式。这里指定了内存拷贝的对齐为 4 字节（即每次拷贝的数据块长度是4字节的倍数）。
    // 这通常与硬件要求或总线宽度相关，确保传输时对齐到4字节边界。
    dma_dev->slave.copy_align                  = DMAENGINE_ALIGN_4_BYTES;

    // 指定DMA设备支持的源地址宽度。这里设置支持的宽度有：
    // 1字节（8位）、2字节（16位）、4字节（32位）。
    // 这些宽度决定了DMA控制器一次能处理的源数据单元大小，常用于配置与外设或内存的接口宽度。
    dma_dev->slave.src_addr_widths             = BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |
                                                 BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |
                                                 BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);

    // 指定DMA设备支持的目标地址宽度。这里也设置了1字节、2字节和4字节的宽度支持。
    // 这与源地址宽度类似，决定了写入到目标地址时的单元大小，可以是与内存或设备通信时的宽度。
    dma_dev->slave.dst_addr_widths             = BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |
                                                 BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |
                                                 BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);

    // 指定DMA支持的传输方向，通常有：
    // 1. DMA_DEV_TO_MEM: 设备到内存传输。常见于从外设（如UART、I2C）读取数据并写入到内存中。
    // 2. DMA_MEM_TO_DEV: 内存到设备传输。常见于从内存将数据写入外设。
    // 这里配置DMA设备支持这两种传输方向（外设到内存和内存到外设）。
    dma_dev->slave.directions                  = BIT(DMA_DEV_TO_MEM) |
                                                 BIT(DMA_MEM_TO_DEV);

    // 设置DMA传输的剩余字节粒度。在 DMA 传输的过程中，驱动程序可能会查询剩余的未传输字节数。
    // 这里设置粒度为DMA_RESIDUE_GRANULARITY_BURST，表示传输剩余数据的精确度是基于DMA传输的突发传输（burst），
    // 而不是单个字节或其他更小的单位。这在高效传输大块数据时很常见。
    dma_dev->slave.residue_granularity          = DMA_RESIDUE_GRANULARITY_BURST;

    // 关联 DMA 设备与平台设备（`pdev`）的设备结构体。这是 DMA 引擎框架中的标准做法，用于
    // 表示这个 DMA 控制器与具体的硬件设备（通过平台设备）相关联。通过 `pdev->dev`，DMA 控制器可以获取
    // 设备相关的信息，例如设备的资源、名称、驱动程序等。
    dma_dev->slave.dev                          = &pdev->dev;

    // 给物理通道数组分配内存
    dma_dev->pchans = devm_kcalloc(&pdev->dev, dma_dev->cfg->max_physical_channels, sizeof(DMA_Physical_Channel_Info), GFP_KERNEL);
    if (!dma_dev->pchans) {
        return -ENOMEM;
    }
    // 给虚拟通道数组分配内存
    dma_dev->vchans = devm_kcalloc(&pdev->dev, dma_dev->cfg->max_virtual_channels, sizeof(DMA_Virtual_Channel_Info), GFP_KERNEL);
    if (!dma_dev->vchans) {
        return -ENOMEM;
    }

    // 初始化物理通道数组
    for (i = 0; i < dma_dev->cfg->max_physical_channels; i++) {
        DMA_Physical_Channel_Info *pchan = &dma_dev->pchans[i];
        pchan->index = i;
        pchan->base_addr = dma_dev->base_addr + DMA_EN_REG_OFFSET(i);

        // 现在是初始化阶段，肯定没任务，置为NULL就对了
        pchan->todo = NULL;
        pchan->done = NULL;
        pchan->vchan = NULL;
    }

    // 初始化虚拟通道数组
    for (i = 0; i < dma_dev->cfg->max_virtual_channels; i++) {
        DMA_Virtual_Channel_Info *vchan = &dma_dev->vchans[i];

        // 同理，现在是初始化阶段，没有任何任务，做一点简单的初始化就行了
        vchan->pchan = NULL;

        vchan->port = 0;
        vchan->irq_type = 0;
        vchan->cyclic = false;
        INIT_LIST_HEAD(&vchan->task_queue_head);
        vchan->task = NULL;

        // 关联描述符的free函数指针，这样内核就可以自动释放DMA描述符了，这里关联任务描述符释放函数，因为任务描述符才是DMA驱动使用，而不是硬件使用的
        vchan->vc.desc_free = hc_free_dma_task_descriptor;

        // 初始化这个虚拟通道
        vchan_init(&vchan->vc, &dma_dev->slave);
    }

    // 初始化任务调度器，用于DMA任务调度，参数为任务结构体指针、调度函数、传给调度函数的参数
    tasklet_init(&dma_dev->task, hc_dma_tasklet, (unsigned long)dma_dev);

    // 解除DMA的复位状态
    ret = reset_control_deassert(dma_dev->rstc);
    if (ret) {
        dev_err(&pdev->dev, "Couldn't deassert the device from reset\n");
        goto cleanup_dma_tasklet;
    }

    // 准备和启用之前申请的给DMA控制器的时钟
    ret = clk_prepare_enable(dma_dev->clk);
    if (ret) {
        dev_err(&pdev->dev, "Couldn't enable the clock\n");
        goto cleanup_dma_reset;
    }

    // 将DMA控制器注册到DMA引擎框架中
    ret = dma_async_device_register(&dma_dev->slave);
    if (ret) {
        dev_warn(&pdev->dev, "Failed to register DMA engine device\n");
        goto cleanup_dma_clock;
    }

    // 向设备树注册 DMA 控制器，允许其他设备通过设备树中的 dma 属性与该控制器通信
    ret = of_dma_controller_register(pdev->dev.of_node, hc_dma_of_xlate, dma_dev);
    if (ret) {
        dev_err(&pdev->dev, "of_dma_controller_register failed\n");
        goto cleanup_dma_engine_registration;
    }

    // 保存设备数据，以便其他代码通过pdev获取dma_dev。
    platform_set_drvdata(pdev, dma_dev);

    return 0;

    // 和上面的逻辑关系类似于栈，这样就能保证所有的操作都能成功回滚
cleanup_dma_engine_registration:
    dma_async_device_unregister(&dma_dev->slave);   // dma_async_device_register的反操作
cleanup_dma_clock:
    clk_disable_unprepare(dma_dev->clk);            // clk_prepare_enable的反操作
cleanup_dma_reset:
    reset_control_assert(dma_dev->rstc);            // reset_control_deassert的反操作
cleanup_dma_tasklet:
    // 这里就是清理之前的所有资源了
    // 内核不能自动回收的，就要在这里清理

    // 首先要禁用该DMA控制器的两组中断使能寄存器，防止它再打中断
    iowrite32(0, dma_dev->base_addr + DMA_IRQ_EN_REG0_OFFSET);
    iowrite32(0, dma_dev->base_addr + DMA_IRQ_EN_REG1_OFFSET);

    // 变更tasklet的运行状态为关闭
    atomic_inc(&dma_dev->tasklet_shutdown);

    // 清理虚拟通道的资源
    for (i = 0; i < dma_dev->cfg->max_virtual_channels; i++) {
        DMA_Virtual_Channel_Info *vchan = &dma_dev->vchans[i];

        // 删掉虚拟通道的设备链表结点
        list_del(&vchan->vc.chan.device_node);

        // 杀掉这个虚拟通道的tasklet，完成销毁
        tasklet_kill(&vchan->vc.task);
    }

    // 杀死该控制器的tasklet，完成所有的tasklet的销毁
    tasklet_kill(&dma_dev->task);

    // 释放申请的IRQ中断，是devm_request_irq的反操作
    devm_free_irq(dma_dev->slave.dev, dma_dev->irq, dma_dev);

    return ret;
}

/*
 * DMA移除函数
 * 功能: 当平台设备被移除时，释放资源并进行清理。
 */
static int hc_dma_remove(struct platform_device *pdev) {
    DMA_DEV_Info *dma_dev = platform_get_drvdata(pdev);
    int i;

    // 首先要禁用该DMA控制器的两组中断使能寄存器，防止它再打中断
    iowrite32(0, dma_dev->base_addr + DMA_IRQ_EN_REG0_OFFSET);
    iowrite32(0, dma_dev->base_addr + DMA_IRQ_EN_REG1_OFFSET);

    // 变更tasklet的运行状态为关闭
    atomic_inc(&dma_dev->tasklet_shutdown);

    // 清理虚拟通道的资源
    for (i = 0; i < dma_dev->cfg->max_virtual_channels; i++) {
        DMA_Virtual_Channel_Info *vchan = &dma_dev->vchans[i];

        // 删掉虚拟通道的设备链表结点
        list_del(&vchan->vc.chan.device_node);

        // 杀掉这个虚拟通道的tasklet，完成销毁
        tasklet_kill(&vchan->vc.task);
    }

    // 杀死该控制器的tasklet，完成所有的tasklet的销毁
    tasklet_kill(&dma_dev->task);

    dma_async_device_unregister(&dma_dev->slave);   // dma_async_device_register的反操作
    clk_disable_unprepare(dma_dev->clk);            // clk_prepare_enable的反操作
    reset_control_assert(dma_dev->rstc);            // reset_control_deassert的反操作

    // 释放申请的IRQ中断，是devm_request_irq的反操作
    devm_free_irq(dma_dev->slave.dev, dma_dev->irq, dma_dev);
}

/*
 * DMA Tasklet函数
 * 功能: 使用tasklet机制处理DMA操作中的任务调度和管理。
 */
static void hc_dma_tasklet(unsigned long data) {
    // 暂时不执行任何操作
}

/*
 * DMA中断处理程序
 * 功能: 处理DMA控制器产生的中断。
 */
static irqreturn_t hc_dma_interrupt(int irq, void *dev_id) {
    return IRQ_NONE;  // 没有处理中断
}

/*
 * DMA描述符准备函数
 * 功能: 为不同类型的DMA操作准备DMA描述符。
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memcpy(
        struct dma_chan *chan, dma_addr_t dest, dma_addr_t src, size_t len, unsigned long flags) {
    return NULL;  // 暂时不支持此操作
}

static struct dma_async_tx_descriptor *hc_dma_prep_slave_sg(
        struct dma_chan *chan, struct scatterlist *sgl, unsigned int sg_len,
        enum dma_transfer_direction dir, unsigned long flags, void *context) {
    return NULL;  // 暂时不支持此操作
}

static struct dma_async_tx_descriptor *hc_dma_prep_dma_cyclic(
        struct dma_chan *chan, dma_addr_t buf_addr, size_t buf_len, size_t period_len,
        enum dma_transfer_direction dir, unsigned long flags) {
    return NULL;  // 暂时不支持此操作
}

/*
 * DMA通道配置函数
 * 功能: 配置DMA通道的相关设置（地址宽度、突发长度等）。
 */
static int hc_dma_config(struct dma_chan *chan, struct dma_slave_config *config) {
    return 0;  // 返回成功
}

/*
 * DMA通道控制函数
 * 功能: 控制DMA通道的暂停、恢复和终止操作。
 */
static int hc_dma_pause(struct dma_chan *chan) {
    return 0;  // 返回成功
}

static int hc_dma_resume(struct dma_chan *chan) {
    return 0;  // 返回成功
}

static int hc_dma_terminate_all(struct dma_chan *chan) {
    return 0;  // 返回成功
}

/*
 * DMA传输状态函数
 * 功能: 获取DMA传输的状态。
 */
static enum dma_status hc_dma_tx_status(
        struct dma_chan *chan, dma_cookie_t cookie, struct dma_tx_state *state) {
    return DMA_COMPLETE;  // 假设传输已完成
}

/*
 * DMA提交待处理函数
 * 功能: 提交待处理的DMA事务。
 */
static void hc_dma_issue_pending(struct dma_chan *chan) {
    // 暂时不处理
}

/*
 * DMA通道分配函数
 * 功能: 为每个虚拟或物理DMA通道分配资源。
 */
static int hc_dma_alloc_chan_resources(struct dma_chan *chan) {
    return 0;  // 成功分配资源
}

/*
 * 释放通道资源函数
 * 功能: 释放与DMA通道相关的资源。
 */
static void hc_dma_free_chan_resources(struct dma_chan *chan) {
    // 暂时不释放任何资源
}

/*
 * DMA传输启动函数
 * 功能: 启动DMA传输操作。
 */
static int hc_dma_start_transfer(struct dma_chan *chan) {
    return 0;  // 成功启动
}

/*
 * DMA传输停止函数
 * 功能: 停止DMA传输操作。
 */
static void hc_dma_stop_transfer(struct dma_chan *chan) {
    // 暂时不处理
}

// 新增函数的实现

// DMA任务描述符释放函数
static void hc_free_dma_task_descriptor(struct virt_dma_desc *vd) {
    // 因为DMA_TASK_Descriptor中，我把vd放在第一个，所以不用container_of的宏，也可以直接强转类型
    // 类似的还有DMA_DEV_Info，DMA_Virtual_Channel_Info

    // vd转描述符指针
    DMA_TASK_Descriptor *task_descriptor = (DMA_TASK_Descriptor *)vd;
    // 转DMA_DEV_Info指针
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(vd->tx.chan->device);

    // 硬件DMA的描述符指针（软件用的指针）
    DMA_HardWare_Descriptor *v_cur = task_descriptor->virtual_addr;
    DMA_HardWare_Descriptor *v_next;
    // 硬件用的指针
    dma_addr_t p_cur = task_descriptor->physical_addr;
    dma_addr_t p_next;

    // 这个函数是清空任务描述符，把任务描述符的链表上挂着的所有DMA硬件描述符全部释放，不是单独释放一个。
    // 所以要循环，释放掉所有DMA硬件描述符
    while(v_cur) {
        v_next = v_cur->v_next_dma_descriptor;
        p_next = v_cur->p_next_dma_descriptor;

        // 释放这条链上硬件描述符使用的内存
        dma_pool_free(dma_dev->pool, v_cur, p_cur);

        // 切换指针
        v_cur = v_next;
        p_cur = p_next;
    }

    // 释放掉当前驱动用的任务描述符的内存
    kfree(task_descriptor);
}

// 该函数从设备树中解析 DMA 请求并返回一个可用的 DMA 通道，同时将通道与指定的端口号关联。
static struct dma_chan *hc_dma_of_xlate(struct of_phandle_args *dma_spec, struct of_dma *ofdma) {
    DMA_DEV_Info *dma_dev = ofdma->of_dma_data;
    struct dma_chan *chan;

    // dma_spec->args[0]存储的是设备树中指定的DMA请求端口
    uint32_t port = dma_spec->args[0];

    // 判断端口是否越界
    if (port > dma_dev->cfg->max_requests) {
        return NULL;
    }

    // 获取任意一个可用的DMA通道
    chan = dma_get_any_slave_channel(&dma_dev->slave);
    if (!chan) {
        return NULL;
    }

    // 虚拟通道对应到该设备的端口，并写入结构体成员中
    ((DMA_Virtual_Channel_Info *)chan)->port = port;

    return chan;
}

// 以下是驱动作者信息和描述、版本、许可证信息

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng, <huangcheng20000702@gmail.com> ");
MODULE_DESCRIPTION("A dma controller driver for OrangePi One");
MODULE_VERSION("1.0");
