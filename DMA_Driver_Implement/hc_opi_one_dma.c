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
#define DMA_EN_REG_OFFSET(N)        (0x100 + 0x40 * (N) + 0x00)     // DMA 通道使能寄存器 (N=0到11)
#define DMA_PAU_REG_OFFSET(N)       (0x100 + 0x40 * (N) + 0x04)     // DMA 通道暂停寄存器 (N=0到11)
#define DMA_DESC_ADDR_REG_OFFSET(N) (0x100 + 0x40 * (N) + 0x08)     // DMA 通道开始地址寄存器 (N=0到11)
#define DMA_CFG_REG_OFFSET(N)       (0x100 + 0x40 * (N) + 0x0C)     // DMA 通道配置寄存器 (N=0到11)
#define DMA_CUR_SRC_REG_OFFSET(N)   (0x100 + 0x40 * (N) + 0x10)     // DMA 通道当前源地址寄存器 (N=0到11)
#define DMA_CUR_DEST_REG_OFFSET(N)  (0x100 + 0x40 * (N) + 0x14)     // DMA 通道当前目标地址寄存器 (N=0到11)
#define DMA_BCNT_LEFT_REG_OFFSET(N) (0x100 + 0x40 * (N) + 0x18)     // DMA 通道剩余字节计数器寄存器 (N=0到11)
#define DMA_PARA_REG_OFFSET(N)      (0x100 + 0x40 * (N) + 0x1C)     // DMA 通道参数寄存器 (N=0到11)
#define DMA_FDESC_ADDR_REG_OFFSET(N)(0x100 + 0x40 * (N) + 0x2C)     // DMA 前描述符地址寄存器 (N=0到11)
#define DMA_PKG_NUM_REG_OFFSET(N)   (0x100 + 0x40 * (N) + 0x30)     // DMA 数据包编号寄存器 (N=0到11)

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
    dma_addr_t physical_addr;               // 描述符的物理地址
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

    struct list_head        pending_node;    // 该虚拟通道的任务等待结点，用于挂到DMA_DEV_Info的pending上
    bool                    need_start;      // 该虚拟通道是否需要启动传输的标志
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
static int hc_dma_start_transfer(DMA_DEV_Info *dma_dev, DMA_Virtual_Channel_Info *vchan);
static void hc_dma_stop_transfer(DMA_DEV_Info *dma_dev, DMA_Virtual_Channel_Info *vchan);

// 扩展新增的函数声明

static struct dma_async_tx_descriptor *hc_dma_prep_dma_xor(struct dma_chan *chan, dma_addr_t dst, dma_addr_t *src, unsigned int src_cnt, size_t len, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_xor_val(struct dma_chan *chan, dma_addr_t *src,	unsigned int src_cnt, size_t len, enum sum_check_flags *result, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_pq(struct dma_chan *chan, dma_addr_t *dst, dma_addr_t *src, unsigned int src_cnt, const unsigned char *scf, size_t len, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_pq_val(struct dma_chan *chan, dma_addr_t *pq, dma_addr_t *src, unsigned int src_cnt, const unsigned char *scf, size_t len, enum sum_check_flags *pqres, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memset(struct dma_chan *chan, dma_addr_t dest, int value, size_t len, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memset_sg(struct dma_chan *chan, struct scatterlist *sg, unsigned int nents, int value, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_interrupt(struct dma_chan *chan, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_sg(struct dma_chan *chan,struct scatterlist *dst_sg, unsigned int dst_nents, struct scatterlist *src_sg, unsigned int src_nents, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_interleaved_dma(struct dma_chan *chan, struct dma_interleaved_template *xt, unsigned long flags);
static struct dma_async_tx_descriptor *hc_dma_prep_dma_imm_data(struct dma_chan *chan, dma_addr_t dst, uint64_t data, unsigned long flags);
static void hc_dma_synchronize(struct dma_chan *chan);

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
    dma_dev->slave.device_prep_dma_xor         = hc_dma_prep_dma_xor;             // 通过异或多个数据块生成校验数据块
    dma_dev->slave.device_prep_dma_xor_val     = hc_dma_prep_dma_xor_val;         // 验证多个数据块之间的异或校验结果
    dma_dev->slave.device_prep_dma_pq          = hc_dma_prep_dma_pq;              // PQ 操作涉及两个校验数据块
    dma_dev->slave.device_prep_dma_pq_val      = hc_dma_prep_dma_pq_val;          // 验证生成的 PQ 校验数据是否正确
    dma_dev->slave.device_prep_dma_memset      = hc_dma_prep_dma_memset;          // 通过 DMA 进行内存的填充操作
    dma_dev->slave.device_prep_dma_memset_sg   = hc_dma_prep_dma_memset_sg;       // 操作的是一个 scatter-gather 列表，用于分散的内存块进行填充
    dma_dev->slave.device_prep_dma_interrupt   = hc_dma_prep_dma_interrupt;       // 当 DMA 操作完成后，通过中断通知处理器
    dma_dev->slave.device_prep_dma_sg          = hc_dma_prep_dma_sg;              // 用于对分散的内存块进行传输
    dma_dev->slave.device_prep_slave_sg        = hc_dma_prep_slave_sg;            // 准备一个从设备的scatter-gather传输
    dma_dev->slave.device_prep_dma_cyclic      = hc_dma_prep_dma_cyclic;          // 准备一个循环DMA操作
    dma_dev->slave.device_prep_interleaved_dma = hc_dma_prep_interleaved_dma;     // 该函数用于准备交错传输，适合复杂数据结构
    dma_dev->slave.device_prep_dma_imm_data    = hc_dma_prep_dma_imm_data;        // 将 8 字节的立即数据传输到目的地址
    dma_dev->slave.device_config               = hc_dma_config;                   // 配置DMA通道
    dma_dev->slave.device_pause                = hc_dma_pause;                    // 暂停DMA传输
    dma_dev->slave.device_resume               = hc_dma_resume;                   // 恢复暂停的DMA传输
    dma_dev->slave.device_terminate_all        = hc_dma_terminate_all;            // 终止所有正在进行的DMA传输
    dma_dev->slave.device_synchronize          = hc_dma_synchronize;              // 确保 DMA 通道在终止操作时所有传输任务安全结束，避免数据丢失或资源争用
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
    // 3. DMA_DEV_TO_DEV: 设备到设备传输。常见于从外设（如UART、I2C）读取数据并写入到其他外设中。
    // 4. DMA_MEM_TO_MEM: 内存到内存传输。就是memcpy。
    // 这里配置DMA设备支持这两种传输方向（外设到内存和内存到外设）。
    dma_dev->slave.directions                  = BIT(DMA_DEV_TO_MEM) |
                                                 BIT(DMA_MEM_TO_DEV) |
                                                 BIT(DMA_DEV_TO_DEV) |
                                                 BIT(DMA_MEM_TO_MEM);

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
        INIT_LIST_HEAD(&vchan->pending_node);
        vchan->need_start = false;

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

    return 0;
}

/*
 * DMA Tasklet函数
 * 功能: 使用tasklet机制处理DMA操作中的任务调度和管理。
 */
static void hc_dma_tasklet(unsigned long data) {

    // data是传入的参数，是结构体指针
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(data);
    // 物理通道指针
    DMA_Physical_Channel_Info *pchan;
    // 虚拟通道指针
    DMA_Virtual_Channel_Info *vchan;
    // 计数器
    uint32_t i;

    // 首先确定哪个虚拟通道为空，将其变为初始化状态
    for(i = 0; i < dma_dev->cfg->max_virtual_channels; i++) {
        vchan = &(dma_dev->vchans[i]);
        // 因为操作的是通道，所以用通道锁保护，也怕中断，所以要用中断自旋锁
        spin_lock_irq(&vchan->vc.lock);
        if(vchan->pchan && vchan->pchan->done) {
            // 说明这条虚拟通道对应的物理通道至少传输过数据，当前虚拟通道的任务才可能已完成
            // 还需进一步判断
            if(vchan_next_desc(&(vchan->vc)) == NULL) {
                // 如果Linux系统的DMA虚拟通道框架认为这条虚拟通道已经没有下一个任务描述符了，说明任务已完成

                // 物理通道和虚拟通道解绑，物理通道空闲（不然没法进行剩下的任务）
                vchan->pchan->vchan = NULL;
                // 清空虚拟通道，虚拟通道初始化状态（不能真初始化，已经注册到虚拟通道框架去了，真初始化会出事，这里说的初始化是删所有能删的数据，vc不能删，其他都可以）
                vchan->pchan = NULL;
                list_del_init(&vchan->pending_node);
                vchan->port = 0;
                vchan->cyclic = 0;
                vchan->irq_type = 0;
                vchan->need_start = false;
                memset(&(vchan->cfg), 0, sizeof(vchan->cfg));
            }
        }
        spin_unlock_irq(&vchan->vc.lock);
    }

    // 任务提交的问题已经在hc_dma_issue_pending中提交上去了
    // hc_dma_issue_pending中把有任务的虚拟通道结点都链到大链表上了

    // 把空闲的物理通道分配给有任务的虚拟通道
    // 有任务，才需要分配，否则都没意义进入这个流程（增加循环进行条件，大链表不为空）

    // 物理通道是设备资源，要用设备锁保护
    spin_lock_irq(&dma_dev->lock);
    for (i = 0; i < dma_dev->cfg->max_physical_channels && !list_empty(&dma_dev->pending); i++) {
        pchan = &(dma_dev->pchans[i]);

        if (pchan->vchan) {
            // 绑定了一条虚拟通道，说明正在执行传输任务
            continue;
        }
        // 进入这个结点对应的vchan
        // 我觉得用contains_of没什么问题，但是保险起见专车专用吧
        vchan = list_first_entry(&dma_dev->pending, DMA_Virtual_Channel_Info, pending_node);

        // 有任务，就绑定两个通道
        pchan->vchan = vchan;
        vchan->pchan = pchan;
        // 然后把pending_node从链表中删掉，这样这个虚拟通道就不会被绑到其他通道上了
        list_del_init(&vchan->pending_node);
        // 需要启动传输任务的标记
        vchan->need_start = true;
    }
    spin_unlock_irq(&dma_dev->lock);

    // 分配了新任务，启动传输才有意义，不然没事找事容易出事
    for(i = 0; i < dma_dev->cfg->max_physical_channels; i++) {
        pchan = &(dma_dev->pchans[i]);
        // 只有物理通道绑定的虚拟通道才需要启动传输
        // 所以缩小范围，从物理通道来找
        vchan = pchan->vchan;

        if(vchan && vchan->need_start) {
            hc_dma_start_transfer(dma_dev, vchan);
        }
    }
}

/*
 * DMA中断处理程序
 * 功能: 处理DMA控制器产生的中断。
 */
static irqreturn_t hc_dma_interrupt(int irq, void *dev_id) {
    // dev_id就是传入的设备信息指针dma_dev
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)dev_id;
    DMA_Physical_Channel_Info *pchan;   // 物理通道指针
    DMA_Virtual_Channel_Info *vchan;    // 虚拟通道指针
    uint32_t reg_value;                 // 寄存器值
    uint32_t ret = IRQ_NONE;            // 返回值，默认为没有IRQ中断
    uint32_t i;                         // 计数器

    // 需要查看的是IRQ挂起寄存器
    // 结构和使能寄存器类似

    // 第一组
    reg_value = ioread32(dma_dev->base_addr + DMA_IRQ_PEND_REG0_OFFSET);
    if(reg_value != 0) {
        // 不等于0，这一组才有需要清除中断的，否则不需要
        // 根据技术手册，写对应位为1即清除中断，写0不变
        // 直接原封不动写回去就行了
        iowrite32(reg_value, dma_dev->base_addr + DMA_IRQ_PEND_REG0_OFFSET);
        // 然后判断是哪个通道的什么中断挂起了，更新状态

        // 参考hc_dma_start_transfer中，清除对应位的做法
        // 这一个寄存器有8个物理通道的状态
        for(i = 0; i < 8; i++) {
            if (reg_value & (7 << (i * 4))) {
                // 只要不是完全等于0就找到打出中断的物理通道下标
                // 明确物理通道下标之后，判断是否需要更新状态
                pchan = &(dma_dev->pchans[i]);
                vchan = pchan->vchan;

                // 要判断是否符合那个通道的中断要求
                // 只要允许的中断类型出现就行，没必要完全符合
                // 所以把值右移，再位与irq_type，只要不为0，说明有符合的
                if( (reg_value >> (i * 4)) & vchan->irq_type ) {
                    // 要判断是否属于循环DMA
                    if(vchan->cyclic) {
                        // 属于循环DMA就要让内核的DMA框架处理了
                        vchan_cyclic_callback(&pchan->todo->vd);
                    } else {
                        // 不属于循环DMA，就需要手动更新任务的状态

                        // 操作通道，自然要上通道锁保护
                        spin_lock(&vchan->vc.lock);
                        // 更新任务状态
                        vchan_cookie_complete(&pchan->todo->vd);
                        // 更新任务指针
                        pchan->done = pchan->todo;
                        pchan->todo = NULL;
                        spin_unlock(&vchan->vc.lock);
                    }
                }
            }
        }

        // 已经处理了一组，尝试调度一次
        if (atomic_read(&dma_dev->tasklet_shutdown) == 0) {
            // 只要没停止，就可以尝试调度
            tasklet_schedule(&dma_dev->task);
        }

        ret = IRQ_HANDLED;
    }

    // 第二组同上
    reg_value = ioread32(dma_dev->base_addr + DMA_IRQ_PEND_REG1_OFFSET);
    if(reg_value != 0) {
        iowrite32(reg_value, dma_dev->base_addr + DMA_IRQ_PEND_REG1_OFFSET);
        // 不同的是这一个寄存器有4个物理通道的状态
        for(i = 0; i < 4; i++) {
            if (reg_value & (7 << (i * 4))) {
                pchan = &(dma_dev->pchans[i + 8]);
                vchan = pchan->vchan;
                if( (reg_value >> (i * 4)) & vchan->irq_type ) {
                    spin_lock(&vchan->vc.lock);
                    vchan_cookie_complete(&pchan->todo->vd);
                    pchan->done = pchan->todo;
                    pchan->todo = NULL;
                    spin_unlock(&vchan->vc.lock);
                }
            }
        }
        if (atomic_read(&dma_dev->tasklet_shutdown) == 0) {
            tasklet_schedule(&dma_dev->task);
        }
        ret = IRQ_HANDLED;
    }

    // 返回结果
    return ret;
}

/*
 * DMA描述符准备函数
 * 功能: 为不同类型的DMA操作准备DMA描述符。
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memcpy(
        struct dma_chan *chan, dma_addr_t dest, dma_addr_t src, size_t len, unsigned long flags) {

    // 获取所需的两个指针，设备信息结构体和虚拟通道结构体
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;

    // 两种DMA描述符

    DMA_TASK_Descriptor *taskDesc;          // 软件（驱动程序）用的描述符
    DMA_HardWare_Descriptor *hardwareDesc;  // 硬件用的描述符

    if(len == 0) {
        // len是0，无效请求，直接返回NULL
        return NULL;
    }

    // 让内核分配软件描述符内存并初始化清空
    taskDesc = kzalloc(sizeof(*taskDesc), GFP_NOWAIT);
    if(!taskDesc) {
        // 这个不用说，失败返回
        return NULL;
    }
    memset(taskDesc, 0, sizeof(*taskDesc));

    // 通过DMA内存池分配内存（DMA内存池只能用来分配硬件描述符注意！）（第三个参数是分配给hardwareDesc的这块内存的物理地址）
    hardwareDesc = dma_pool_alloc(dma_dev->pool, GFP_NOWAIT, &(taskDesc->physical_addr));
    if (!hardwareDesc) {
        dev_err(dma_dev->slave.dev, "Failed to alloc DMA_HardWare_Descriptor memory\n");
        // 失败了就把任务描述符的内存释放了返回NULL
        kfree(taskDesc);
        return NULL;
    }
    memset(hardwareDesc, 0, sizeof(*hardwareDesc));

    // 把软件描述符的参数补完
    // 物理地址已经填上了，所以这里只需要补软件地址就行
    taskDesc->virtual_addr = hardwareDesc;

    // 配置硬件描述符各项参数
    hardwareDesc->src.DMA_CUR_SRC_ADDR = src;
    hardwareDesc->dst.DMA_CUR_DEST_ADDR = dest;
    hardwareDesc->len = len;
    hardwareDesc->para.WAIT_CYC = 8;            // 说明一下，按照参考驱动的源代码，普通等待时间被设置为8，这边有8位，范围就是 0 到 0xff = 255，不敢改，那就还是8吧
    hardwareDesc->p_next_dma_descriptor = DMA_DESCRIPTOR_END_ADDRESS;   // 当然没有下一个任务了
    hardwareDesc->v_next_dma_descriptor = NULL; // 同上

    hardwareDesc->cfg.DMA_SRC_DRQ_TYPE = 1;     // memcpy本来就只需要支持内存到内存，SDRAM是1，这个在数据手册p191页的表格4-1可以看到
    hardwareDesc->cfg.DMA_DEST_DRQ_TYPE = 1;    // 同上
    hardwareDesc->cfg.DMA_SRC_ADDR_MODE = 0;    // 内存当然是线性模式，这不用解释
    hardwareDesc->cfg.DMA_DEST_ADDR_MODE = 0;   // 同上
    hardwareDesc->cfg.DMA_SRC_BST_LEN = 2;      // 参考代码统一突发长度为8
    hardwareDesc->cfg.DMA_DEST_BST_LEN = 2;     // 同上
    hardwareDesc->cfg.DMA_SRC_DATA_WIDTH = 2;   // 源设备的总线位宽，32位内存当然是32
    hardwareDesc->cfg.DMA_DEST_DATA_WIDTH = 2;  // 同上

    // 让Linux的DMA驱动框架完成后续配置，这里不管了
    return vchan_tx_prep(&vchan->vc, &taskDesc->vd, flags);
}

static struct dma_async_tx_descriptor *hc_dma_prep_slave_sg(
        struct dma_chan *chan, struct scatterlist *sgl, unsigned int sg_len,
        enum dma_transfer_direction dir, unsigned long flags, void *context) {

    // 其实它们都类似，只是描述符的配置不太相同
    // 本质上来说，真正的差异还是描述符的各项配置
    // 因此可以直接从memcpy里面复制代码下来改

    // 获取所需的两个指针，设备信息结构体和虚拟通道结构体
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;

    // 两种DMA描述符

    DMA_TASK_Descriptor *taskDesc;          // 软件（驱动程序）用的描述符
    DMA_HardWare_Descriptor *hardwareDesc;  // 硬件用的描述符

    DMA_HardWare_Descriptor *tail = NULL;   // 为了串到链表尾结点用到

    DMA_HardWare_Descriptor *temp_hardwareDesc; // 销毁描述符的时候用到的变量

    // 描述符物理地址
    dma_addr_t physical_addr;

    // sg链表结点
    struct scatterlist *sg_node;

    // 计数器
    uint32_t i;

    // 通道参数，每个描述符都用得到
    uint32_t src_width;
    uint32_t dest_width;
    uint32_t src_burst;
    uint32_t dest_burst;

    if(!dma_dev || !vchan || !sgl || sg_len == 0) {
        // 取不到设备信息、取不到虚拟通道、链表不存在或者len是0，就是无效请求，直接返回NULL
        return NULL;
    }

    // 在这里检查通道参数是否有效
    switch (vchan->cfg.src_addr_width) {                        // 源设备位宽（根据DMA_CFG_REG_t 结构体的DMA_SRC_DATA_WIDTH 项要求）
        case DMA_SLAVE_BUSWIDTH_1_BYTE: {
            src_width = 0;  // 1字节就是8位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_2_BYTES: {
            src_width = 1;  // 2字节就是16位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_4_BYTES: {
            src_width = 2;   // 4字节就是32位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_8_BYTES: {
            src_width = 3;   // 8字节就是64位
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }
    switch (vchan->cfg.dst_addr_width) {                        // 目标设备位宽（根据DMA_CFG_REG_t 结构体的DMA_DEST_DATA_WIDTH 项要求）
        case DMA_SLAVE_BUSWIDTH_1_BYTE: {
            dest_width = 0; // 1字节就是8位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_2_BYTES: {
            dest_width = 1; // 2字节就是16位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_4_BYTES: {
            dest_width = 2;  // 4字节就是32位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_8_BYTES: {
            dest_width = 3;  // 8字节就是64位
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }
    switch (vchan->cfg.src_maxburst) {                          // 源设备突发长度（根据DMA_CFG_REG_t 结构体的DMA_SRC_BST_LEN 项要求）
        case 1: {
            src_burst = 0;
            break;
        }
        case 4: {
            src_burst = 1;
            break;
        }
        case 8: {
            src_burst = 2;
            break;
        }
        case 16: {
            src_burst = 3;
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }
    switch (vchan->cfg.dst_maxburst) {                          // 目标设备突发长度（根据DMA_CFG_REG_t 结构体的DMA_DEST_BST_LEN 项要求）
        case 1: {
            dest_burst = 0;
            break;
        }
        case 4: {
            dest_burst = 1;
            break;
        }
        case 8: {
            dest_burst = 2;
            break;
        }
        case 16: {
            dest_burst = 3;
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }

    // 让内核分配软件描述符内存并初始化清空
    taskDesc = kzalloc(sizeof(*taskDesc), GFP_NOWAIT);
    if(!taskDesc) {
        // 这个不用说，失败返回
        return NULL;
    }
    memset(taskDesc, 0, sizeof(*taskDesc));

    // 通过scatterlist提供的的遍历宏来遍历所有链表（最好专车专用，其实自己钻到底层写while循环也行）
    for_each_sg(sgl, sg_node, sg_len, i) {
        // 对于每个结点，就创建一个描述符
        hardwareDesc = dma_pool_alloc(dma_dev->pool, GFP_NOWAIT, &physical_addr);
        if (!hardwareDesc) {
            dev_err(dma_dev->slave.dev, "Failed to alloc DMA_HardWare_Descriptor memory\n");
            // 失败了就把所有硬件描述符的内存释放了

            // 第一个描述符记录在taskDesc中
            physical_addr = taskDesc->physical_addr;
            hardwareDesc = taskDesc->virtual_addr;
            temp_hardwareDesc = hardwareDesc;

            while(temp_hardwareDesc) {
                dma_pool_free(dma_dev->pool, hardwareDesc, physical_addr);
                physical_addr = temp_hardwareDesc->p_next_dma_descriptor;
                hardwareDesc = temp_hardwareDesc->v_next_dma_descriptor;
                // 链表结点步进
                temp_hardwareDesc = hardwareDesc;
            }
            // 再释放任务描述符
            kfree(taskDesc);
            // 返回NULL
            return NULL;
        }
        memset(hardwareDesc, 0, sizeof(*hardwareDesc));

        // 把软件描述符中的结点信息补上，这是第一个结点
        if(taskDesc->virtual_addr == NULL) {
            taskDesc->physical_addr = physical_addr;
            taskDesc->virtual_addr = hardwareDesc;
        }

        if(tail) {
            // 不是第一个结点，需要串到链表末尾
            tail->p_next_dma_descriptor = physical_addr;
            tail->v_next_dma_descriptor = hardwareDesc;
        }
        // 以下是无论如何都要做的

        tail = hardwareDesc;    // 更新尾结点
        hardwareDesc->p_next_dma_descriptor = DMA_DESCRIPTOR_END_ADDRESS;   // 新的尾结点当然没有下一个任务了
        hardwareDesc->v_next_dma_descriptor = NULL; // 同上

        // 根据方向配置硬件描述符各项参数（注意要通过宏提取sg链表结点里面的信息，这是比较推荐的做法，否则需要自己区分各种情况，很麻烦）

        switch (dir) {
            case DMA_MEM_TO_DEV: {
                // 源地址是 scatterlist 中的地址，目标地址是设备的地址
                hardwareDesc->src.DMA_CUR_SRC_ADDR = sg_dma_address(sg_node);
                hardwareDesc->len = sg_dma_len(sg_node);

                hardwareDesc->para.WAIT_CYC = 8;            // 说明一下，按照参考驱动的源代码，普通等待时间被设置为8，这边有8位，范围就是 0 到 0xff = 255，不敢改，那就还是8吧

                // 从 dma_slave_config 结构体中提取信息
                hardwareDesc->dst.DMA_CUR_DEST_ADDR = vchan->cfg.dst_addr;  // 目标地址


                hardwareDesc->cfg.DMA_SRC_DRQ_TYPE = 1;                 // 内存到设备，源设备肯定是内存，根据DRQ表，当然是1
                hardwareDesc->cfg.DMA_DEST_DRQ_TYPE = vchan->port;      // 具体看hc_dma_of_xlate写入的是什么

                hardwareDesc->cfg.DMA_SRC_ADDR_MODE = 0;    // 源设备内存当然是线性模式，这不用解释
                hardwareDesc->cfg.DMA_DEST_ADDR_MODE = 1;   // 目标设备不知道是什么，一律当作IO模式处理

                break;
            }
            case DMA_DEV_TO_MEM: {
                // 源地址是设备的地址，目标地址是 scatterlist 中的地址
                hardwareDesc->dst.DMA_CUR_DEST_ADDR = sg_dma_address(sg_node);
                hardwareDesc->len = sg_dma_len(sg_node);

                // 从 dma_slave_config 结构体中提取信息
                hardwareDesc->src.DMA_CUR_SRC_ADDR = vchan->cfg.src_addr;   // 目标地址

                hardwareDesc->para.WAIT_CYC = 8;            // 说明一下，按照参考驱动的源代码，普通等待时间被设置为8，这边有8位，范围就是 0 到 0xff = 255，不敢改，那就还是8吧

                hardwareDesc->cfg.DMA_SRC_DRQ_TYPE = vchan->port;       // 具体看hc_dma_of_xlate写入的是什么
                hardwareDesc->cfg.DMA_DEST_DRQ_TYPE = 1;                // 设备到内存，源设备肯定是内存，根据DRQ表，当然是1

                hardwareDesc->cfg.DMA_SRC_ADDR_MODE = 1;    // 源设备不知道是什么，一律当作IO模式处理
                hardwareDesc->cfg.DMA_DEST_ADDR_MODE = 0;   // 目标设备内存当然是线性模式，这不用解释

                break;
            }
            default: {
                // 目前不支持其他情况，只支持这两个
                // 我一直觉得该有设备到设备，但是原来的参考代码就没支持
                break;
            }
        }

        // 这些是方向无关的参数
        hardwareDesc->cfg.DMA_SRC_DATA_WIDTH = src_width;
        hardwareDesc->cfg.DMA_DEST_DATA_WIDTH = dest_width;
        hardwareDesc->cfg.DMA_SRC_BST_LEN = src_burst;
        hardwareDesc->cfg.DMA_DEST_BST_LEN = dest_burst;

    }

    // 让Linux的DMA驱动框架完成后续配置，这里不管了
    return vchan_tx_prep(&vchan->vc, &taskDesc->vd, flags);
}

static struct dma_async_tx_descriptor *hc_dma_prep_dma_cyclic(
        struct dma_chan *chan, dma_addr_t buf_addr, size_t buf_len, size_t period_len,
        enum dma_transfer_direction dir, unsigned long flags) {

    // 循环DMA其实类似于连续的hc_dma_prep_slave_sg
    // 它的地址是连续计算出来的

    // 获取所需的两个指针，设备信息结构体和虚拟通道结构体
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;

    // 两种DMA描述符

    DMA_TASK_Descriptor *taskDesc;          // 软件（驱动程序）用的描述符
    DMA_HardWare_Descriptor *hardwareDesc;  // 硬件用的描述符

    DMA_HardWare_Descriptor *tail = NULL;   // 为了串到链表尾结点用到

    DMA_HardWare_Descriptor *temp_hardwareDesc; // 销毁描述符的时候用到的变量

    // 描述符物理地址
    dma_addr_t physical_addr;

    // 计数器
    uint32_t i;

    // 计算总共要多少次（它是发定长数据，直到发完为止）
    uint32_t count = buf_len / period_len;

    // 通道参数，每个描述符都用得到
    uint32_t src_width;
    uint32_t dest_width;
    uint32_t src_burst;
    uint32_t dest_burst;

    if(!dma_dev || !vchan || buf_addr == 0 || buf_len == 0 || period_len == 0) {
        // 取不到设备信息、取不到虚拟通道、链表不存在或者len是0，就是无效请求，直接返回NULL
        return NULL;
    }

    // 在这里检查通道参数是否有效
    switch (vchan->cfg.src_addr_width) {                        // 源设备位宽（根据DMA_CFG_REG_t 结构体的DMA_SRC_DATA_WIDTH 项要求）
        case DMA_SLAVE_BUSWIDTH_1_BYTE: {
            src_width = 0;  // 1字节就是8位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_2_BYTES: {
            src_width = 1;  // 2字节就是16位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_4_BYTES: {
            src_width = 2;   // 4字节就是32位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_8_BYTES: {
            src_width = 3;   // 8字节就是64位
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }
    switch (vchan->cfg.dst_addr_width) {                        // 目标设备位宽（根据DMA_CFG_REG_t 结构体的DMA_DEST_DATA_WIDTH 项要求）
        case DMA_SLAVE_BUSWIDTH_1_BYTE: {
            dest_width = 0; // 1字节就是8位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_2_BYTES: {
            dest_width = 1; // 2字节就是16位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_4_BYTES: {
            dest_width = 2;  // 4字节就是32位
            break;
        }
        case DMA_SLAVE_BUSWIDTH_8_BYTES: {
            dest_width = 3;  // 8字节就是64位
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }
    switch (vchan->cfg.src_maxburst) {                          // 源设备突发长度（根据DMA_CFG_REG_t 结构体的DMA_SRC_BST_LEN 项要求）
        case 1: {
            src_burst = 0;
            break;
        }
        case 4: {
            src_burst = 1;
            break;
        }
        case 8: {
            src_burst = 2;
            break;
        }
        case 16: {
            src_burst = 3;
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }
    switch (vchan->cfg.dst_maxburst) {                          // 目标设备突发长度（根据DMA_CFG_REG_t 结构体的DMA_DEST_BST_LEN 项要求）
        case 1: {
            dest_burst = 0;
            break;
        }
        case 4: {
            dest_burst = 1;
            break;
        }
        case 8: {
            dest_burst = 2;
            break;
        }
        case 16: {
            dest_burst = 3;
            break;
        }
        default: {
            // 其他情况无效，直接返回
            return NULL;
        }
    }

    // 让内核分配软件描述符内存并初始化清空
    taskDesc = kzalloc(sizeof(*taskDesc), GFP_NOWAIT);
    if(!taskDesc) {
        // 这个不用说，失败返回
        return NULL;
    }
    memset(taskDesc, 0, sizeof(*taskDesc));

    for(i = 0; i < count; i++) {
        // 对于每个结点，就创建一个描述符
        hardwareDesc = dma_pool_alloc(dma_dev->pool, GFP_NOWAIT, &physical_addr);
        if (!hardwareDesc) {
            dev_err(dma_dev->slave.dev, "Failed to alloc DMA_HardWare_Descriptor memory\n");
            // 失败了就把所有硬件描述符的内存释放了

            // 第一个描述符记录在taskDesc中
            physical_addr = taskDesc->physical_addr;
            hardwareDesc = taskDesc->virtual_addr;
            temp_hardwareDesc = hardwareDesc;

            while(temp_hardwareDesc) {
                dma_pool_free(dma_dev->pool, hardwareDesc, physical_addr);
                physical_addr = temp_hardwareDesc->p_next_dma_descriptor;
                hardwareDesc = temp_hardwareDesc->v_next_dma_descriptor;
                // 链表结点步进
                temp_hardwareDesc = hardwareDesc;
            }
            // 再释放任务描述符
            kfree(taskDesc);
            // 返回NULL
            return NULL;
        }
        memset(hardwareDesc, 0, sizeof(*hardwareDesc));

        // 把软件描述符中的结点信息补上，这是第一个结点
        if(taskDesc->virtual_addr == NULL) {
            taskDesc->physical_addr = physical_addr;
            taskDesc->virtual_addr = hardwareDesc;
        }

        if(tail) {
            // 不是第一个结点，需要串到链表末尾
            tail->p_next_dma_descriptor = physical_addr;
            tail->v_next_dma_descriptor = hardwareDesc;
        }
        // 以下是无论如何都要做的

        tail = hardwareDesc;    // 更新尾结点
        hardwareDesc->p_next_dma_descriptor = DMA_DESCRIPTOR_END_ADDRESS;   // 新的尾结点当然没有下一个任务了
        hardwareDesc->v_next_dma_descriptor = NULL; // 同上

        // 根据方向配置硬件描述符各项参数

        switch (dir) {
            case DMA_MEM_TO_DEV: {
                // 源地址是buf_addr + offset，目标地址是设备的地址
                hardwareDesc->src.DMA_CUR_SRC_ADDR = buf_addr + i * period_len;
                hardwareDesc->len = period_len;

                hardwareDesc->para.WAIT_CYC = 8;            // 说明一下，按照参考驱动的源代码，普通等待时间被设置为8，这边有8位，范围就是 0 到 0xff = 255，不敢改，那就还是8吧

                // 从 dma_slave_config 结构体中提取信息
                hardwareDesc->dst.DMA_CUR_DEST_ADDR = vchan->cfg.dst_addr;  // 目标地址


                hardwareDesc->cfg.DMA_SRC_DRQ_TYPE = 1;                 // 内存到设备，源设备肯定是内存，根据DRQ表，当然是1
                hardwareDesc->cfg.DMA_DEST_DRQ_TYPE = vchan->port;      // 具体看hc_dma_of_xlate写入的是什么

                hardwareDesc->cfg.DMA_SRC_ADDR_MODE = 0;    // 源设备内存当然是线性模式，这不用解释
                hardwareDesc->cfg.DMA_DEST_ADDR_MODE = 1;   // 目标设备不知道是什么，一律当作IO模式处理

                break;
            }
            case DMA_DEV_TO_MEM: {
                // 源地址是设备的地址，目标地址是 buf_addr + offset
                hardwareDesc->dst.DMA_CUR_DEST_ADDR = buf_addr + i * period_len;
                hardwareDesc->len = period_len;

                // 从 dma_slave_config 结构体中提取信息
                hardwareDesc->src.DMA_CUR_SRC_ADDR = vchan->cfg.src_addr;   // 目标地址

                hardwareDesc->para.WAIT_CYC = 8;            // 说明一下，按照参考驱动的源代码，普通等待时间被设置为8，这边有8位，范围就是 0 到 0xff = 255，不敢改，那就还是8吧

                hardwareDesc->cfg.DMA_SRC_DRQ_TYPE = vchan->port;       // 具体看hc_dma_of_xlate写入的是什么
                hardwareDesc->cfg.DMA_DEST_DRQ_TYPE = 1;                // 设备到内存，源设备肯定是内存，根据DRQ表，当然是1

                hardwareDesc->cfg.DMA_SRC_ADDR_MODE = 1;    // 源设备不知道是什么，一律当作IO模式处理
                hardwareDesc->cfg.DMA_DEST_ADDR_MODE = 0;   // 目标设备内存当然是线性模式，这不用解释

                break;
            }
            default: {
                // 目前不支持其他情况，只支持这两个
                // 我一直觉得该有设备到设备，但是原来的参考代码就没支持
                break;
            }
        }

        // 这些是方向无关的参数
        hardwareDesc->cfg.DMA_SRC_DATA_WIDTH = src_width;
        hardwareDesc->cfg.DMA_DEST_DATA_WIDTH = dest_width;
        hardwareDesc->cfg.DMA_SRC_BST_LEN = src_burst;
        hardwareDesc->cfg.DMA_DEST_BST_LEN = dest_burst;
    }

    // 这里是循环DMA的特殊处理

    // 把DMA描述符链表直接回环
    tail->p_next_dma_descriptor = taskDesc->physical_addr;
    // 不要回环驱动用的 v_next_dma_descriptor，那样删除链表的时候只会引起不必要的麻烦

    // 通道的循环属性打上
    vchan->cyclic = true;

    // 让Linux的DMA驱动框架完成后续配置，这里不管了
    return vchan_tx_prep(&vchan->vc, &taskDesc->vd, flags);
}

/*
 * DMA通道配置函数
 * 功能: 配置DMA通道的相关设置（地址宽度、突发长度等）。
 */
static int hc_dma_config(struct dma_chan *chan, struct dma_slave_config *config) {
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;
    memcpy(&vchan->cfg, config, sizeof(*config));
    return 0;  // 返回成功
}

/*
 * DMA通道控制函数
 * 功能: 控制DMA通道的暂停、恢复和终止操作。
 */
static int hc_dma_pause(struct dma_chan *chan) {
    // 需要明确的是：
    // 1. 操作的通道对象实际上是虚拟通道
    // 2. 既然是虚拟通道，那就要分是否有物理通道（正在进行传输）和没有物理通道（等待进行传输）的情况
    // 3. 有物理通道，那自然就要写物理通道寄存器（既然是暂停，任务还是要继续的，所以不能解绑物理通道和虚拟通道，不缺这点资源）
    // 4. 没有物理通道，那就直接把该通道从等待队列里面删掉，不需要再分配物理通道了
    // 对于4.这种情况来说，考虑到tasklet中，分配物理通道的判断条件是!list_empty(&dma_dev->pending)，是从dma_dev->pending里面取对应的有任务的物理通道
    // 所以直接把这个结点从大链表中删掉就行了

    // 获取设备信息和通道的指针
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;
    DMA_Physical_Channel_Info *pchan = vchan->pchan;
    uint32_t reg_value;
    // 这样就可以用结构体直接写寄存器值的特定位了
    DMA_PAU_REG_t *dmaPauReg = (DMA_PAU_REG_t *)(&reg_value);

    if(pchan) {
        // 读对应的物理通道寄存器
        reg_value = ioread32(dma_dev->base_addr + DMA_PAU_REG_OFFSET(pchan->index));
        // 修改暂停位，暂停传输
        dmaPauReg->DMA_PAUSE = 1;
        // 写回寄存器
        iowrite32(reg_value, dma_dev->base_addr + DMA_PAU_REG_OFFSET(pchan->index));

    } else {
        // 操作的是大链表，自然要用设备锁保护
        spin_lock(&dma_dev->lock);
        // 直接把这个通道的结点从大链表中删掉就行了
        list_del_init(&vchan->pending_node);

        spin_unlock(&dma_dev->lock);
    }

    return 0;  // 返回成功
}

static int hc_dma_resume(struct dma_chan *chan) {
    // hc_dma_pause的反操作，反过来就行了

    // 获取设备信息和通道的指针
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;
    DMA_Physical_Channel_Info *pchan = vchan->pchan;
    uint32_t reg_value;
    // 这样就可以用结构体直接写寄存器值的特定位了
    DMA_PAU_REG_t *dmaPauReg = (DMA_PAU_REG_t *)(&reg_value);

    if(pchan) {
        // 读对应的物理通道寄存器
        reg_value = ioread32(dma_dev->base_addr + DMA_PAU_REG_OFFSET(pchan->index));
        // 修改暂停位，恢复传输
        dmaPauReg->DMA_PAUSE = 0;
        // 写回寄存器
        iowrite32(reg_value, dma_dev->base_addr + DMA_PAU_REG_OFFSET(pchan->index));

    } else {
        // 操作的是大链表，自然要用设备锁保护
        spin_lock(&dma_dev->lock);
        // 直接把这个通道的结点加回到大链表中就行了
        list_add_tail(&vchan->pending_node, &dma_dev->pending);

        spin_unlock(&dma_dev->lock);
    }

    return 0;  // 返回成功
}

static int hc_dma_terminate_all(struct dma_chan *chan) {

    // 获取设备信息和通道的指针
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;
    DMA_Physical_Channel_Info *pchan = vchan->pchan;
    // 要直接操作虚拟通道信息，用到spin_lock_irqsave，因此需要保存上下文的变量
    unsigned long flags;

    uint32_t reg_value;
    // 这样就可以用结构体直接写寄存器值的特定位了
    DMA_EN_REG_t *dmaEnReg = (DMA_EN_REG_t *)(&reg_value);
    DMA_PAU_REG_t *dmaPauReg = (DMA_PAU_REG_t *)(&reg_value);

    // 创建一个头结点，用于获取虚拟通道的任务描述符链表，并将其销毁
    LIST_HEAD(desc_head);

    if(pchan) {

        // 要变更虚拟通道资源，所以需要通道锁保护
        spin_lock_irqsave(&vchan->vc.lock, flags);

        // 如果物理通道已经绑定，写寄存器以终止传输
        // 首先恢复传输（如果先前是暂停状态，恢复传输才可以真正停止），然后禁用通道传输功能
        // 如此才能真正停止传输

        // 该通道恢复传输
        reg_value = ioread32(dma_dev->base_addr + DMA_PAU_REG_OFFSET(pchan->index));
        dmaPauReg->DMA_PAUSE = 0;
        iowrite32(reg_value, dma_dev->base_addr + DMA_PAU_REG_OFFSET(pchan->index));

        // 禁用物理通道（hc_dma_start_transfer中会启用，所以不用担心）
        reg_value = ioread32(dma_dev->base_addr + DMA_EN_REG_OFFSET(pchan->index));
        dmaEnReg->DMA_EN = 0;
        iowrite32(reg_value, dma_dev->base_addr + DMA_EN_REG_OFFSET(pchan->index));

        // 对于循环DMA通道，需要特殊处理
        if (vchan->cyclic) {
            // 循环的属性去掉
            vchan->cyclic = false;

            // 将已完成的任务加入到虚拟通道的已完成描述符链表中
            // 这是因为循环DMA不会自动完成传输（这是其特性）
            // 必须手动标记已完成的任务，以便框架正确处理和释放资源
            if(pchan->todo) {
                list_add_tail(&(pchan->todo->vd.node), &(vchan->vc.desc_completed));
            }
        }

        // 清理物理通道资源
        pchan->todo = NULL;
        pchan->done = NULL;
        // 解绑物理通道和虚拟通道
        pchan->vchan = NULL;
        vchan->pchan = NULL;

        spin_unlock_irqrestore(&vchan->vc.lock, flags);

    } else {
        // 没任务可传输，直接从大链表中删掉
        spin_lock(&dma_dev->lock);
        list_del_init(&vchan->pending_node);
        spin_unlock(&dma_dev->lock);
    }

    // 然后回收任务描述符

    // 属于虚拟通道的资源，需要通道锁保护
    spin_lock_irqsave(&vchan->vc.lock, flags);

    // 把所有的描述符转移到新链表头结点上（Linux内核虚拟通道支持框架提供的函数）
    vchan_get_all_descriptors(&vchan->vc, &desc_head);


    spin_unlock_irqrestore(&vchan->vc.lock, flags);

    // 不属于虚拟通道之后，就可以不用通道锁保护了

    // 让虚拟DMA通道框架释放描述符链表上的所有描述符
    vchan_dma_desc_free_list(&vchan->vc, &desc_head);

    return 0;  // 返回成功
}

/*
 * DMA传输状态函数
 * 功能: 获取DMA传输的状态。
 */
static enum dma_status hc_dma_tx_status(
        struct dma_chan *chan, dma_cookie_t cookie, struct dma_tx_state *state) {

    // 获取所需的指针，虚拟通道结构体和物理通道结构体
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;
    DMA_Physical_Channel_Info *pchan = vchan->pchan;

    // 任务描述符和硬件描述符
    DMA_TASK_Descriptor *taskDesc;
    DMA_HardWare_Descriptor *hardwareDesc;

    unsigned long flags;    // 自旋锁保存中断上下文的变量
    size_t bytes;

    enum dma_status res;    // 返回结果，也就是DMA传输状态

    res = dma_cookie_status(chan, cookie, state);
    if (res == DMA_COMPLETE || !state) {
        // 已经有结果了直接返回
        return res;
    }
    // 这里是更新状态，状态是会变的，可能会被中断打断
    // 所以这里要加锁保护，保护这个状态更新过程（其实是bytes的计算过程）
    // 保护的粒度是这个虚拟通道，所以不能用设备锁
    spin_lock_irqsave(&vchan->vc.lock, flags);

    // 用框架给的函数找LinuxDMA引擎用的DMA任务描述符，将其转换为驱动程序里用的DMA_TASK_Descriptor
    taskDesc = (DMA_TASK_Descriptor *)vchan_find_desc(&vchan->vc, cookie);
    if(taskDesc == NULL) {
        // 找不到的话需要确认一下情况（一种是传输完毕，一种是正在传输）

        // 读当前通道开始地址寄存器，如果到达终点了就是描述符链结束标志
        if(ioread32(((DMA_DEV_Info *)(chan->device))->base_addr + DMA_DESC_ADDR_REG_OFFSET(pchan->index)) == DMA_DESCRIPTOR_END_ADDRESS) {
            // 都结束了，当然是0
            bytes = 0;
        } else {
            // 说明一下，通道剩余字节数量寄存器，那个显示的是当前这个数据包传输还剩多少个字节，不是该任务所有数据包
            bytes = ioread32(((DMA_DEV_Info *)(chan->device))->base_addr + DMA_BCNT_LEFT_REG_OFFSET(pchan->index));

            // 计算一下这批任务还剩多少个数据包，每个数据包多少个字节，加起来就是了
            hardwareDesc = taskDesc->virtual_addr;
            while(hardwareDesc != NULL) {
                bytes += hardwareDesc->len;
                hardwareDesc = hardwareDesc->v_next_dma_descriptor;
            }
        }

    } else {
        // 找得到意味着还没开始传输
        bytes = 0;
        // 直接计算一下这批任务还剩多少个数据包，每个数据包多少个字节，加起来就是了
        hardwareDesc = taskDesc->virtual_addr;
        while(hardwareDesc != NULL) {
            bytes += hardwareDesc->len;
            hardwareDesc = hardwareDesc->v_next_dma_descriptor;
        }
    }

    spin_unlock_irqrestore(&vchan->vc.lock, flags);
    dma_set_residue(state, bytes);  // 设置DMA传输操作中剩余未传输的数据量，就能更新状态了
    return res;
}

/*
 * DMA提交待处理函数
 * 功能: 提交待处理的DMA事务。
 */
static void hc_dma_issue_pending(struct dma_chan *chan) {
    // 获取所需的两个指针，设备信息结构体和虚拟通道结构体
    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;
    // 自旋锁保存中断上下文所需的变量
    unsigned long flags;

    // 提交的对象是提交到哪条通道，所以要用通道锁保护
    spin_lock_irqsave(&vchan->vc.lock, flags);

    // 本质上是提交到Linux内核的DMA虚拟通道框架里面，让Linux去处理实际的事务过程
    if (vchan_issue_pending(&vchan->vc)) {
        // 因为要添加到大链表，也就是设备信息结构体的pending上，所以需要设备锁保护
        spin_lock(&dma_dev->lock);
        // 把当前这个通道的任务链表加到大链表末尾
        list_add_tail(&vchan->pending_node, &dma_dev->pending);
        // 这里是用tasklet_schedule来强行调度一次，不然没中断的话就永远调度不了了
        tasklet_schedule(&dma_dev->task);

        spin_unlock(&dma_dev->lock);
    } else {
        // 这种情况不正常，这是属于没提交事务的情况，不应该发生，要用dev_err打一下哪条虚拟通道出问题了
        dev_err(chan->device->dev, "DMA issue pending failed on virtual channel: %d\n", vchan->vc.chan.chan_id);
    }

    spin_unlock_irqrestore(&vchan->vc.lock, flags);
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
    // 获取所需的两个指针，设备信息结构体和虚拟通道结构体

    DMA_DEV_Info *dma_dev = (DMA_DEV_Info *)(chan->device);
    DMA_Virtual_Channel_Info *vchan = (DMA_Virtual_Channel_Info *)chan;

    // 这是保存上下文的变量
    unsigned long flags;

    // 用设备信息结构体的锁（锁的是整个设备）来保护操作，因为可能进入中断状态，所以要保存中断状态上下文
    spin_lock_irqsave(&dma_dev->lock, flags);
    list_del_init(&vchan->pending_node);         // 进行的操作就是把这个虚拟通道的任务队列结点从大链表中删掉，具体来说就是DMA_DEV_Info的pending
    spin_unlock_irqrestore(&dma_dev->lock, flags);

    // 释放虚拟通道的全部资源，其实也可以自己来
    vchan_free_chan_resources(&vchan->vc);
}

/*
 * DMA传输启动函数
 * 功能: 启动DMA传输操作。
 */
static int hc_dma_start_transfer(DMA_DEV_Info *dma_dev, DMA_Virtual_Channel_Info *vchan) {

    struct virt_dma_desc *desc = vchan_next_desc(&vchan->vc);
    DMA_Physical_Channel_Info *pchan;
    uint32_t reg_value;

    // 先判断，怕出事
    if(!dma_dev || !vchan || !desc || !(vchan->need_start)) {
        return -EAGAIN;
    }

    // 涉及这个通道的操作，所以需要用通道锁保护
    spin_lock_irq(&vchan->vc.lock);

    // 首先把通道的标记改了
    vchan->need_start = false;

    // 再把这个任务结点从虚拟DMA框架的任务描述符里面删了（不然虚拟DMA框架认为这个任务还是这个虚拟通道的任务）
    list_del(&desc->node);

    pchan = vchan->pchan;
    pchan->todo = (DMA_TASK_Descriptor *)desc;
    pchan->done = NULL;

    // 按照参考的驱动代码的做法
    // IRQ类型是根据是否循环DMA来确定的，如果是循环DMA，就单开全包中断，如果不是循环DMA，就单开队列结束中断，也就是如下
    // vchan->irq_type = vchan->cyclic ? DMA_IRQ_PKG : DMA_IRQ_QUEUE;
    // 似乎没考虑半包中断，那我也不用考虑了，仿照就是了，后面可能尝试实现半包中断

    // 半包是位0，全包是位1，队列是位2
    // 单开全包是1 << 0 = 1，单开全包是 1 << 1 = 2，单开队列是1 << 2 = 4
    vchan->irq_type = vchan->cyclic ? 2 : 4;

    // 开启对应通道的IRQ中断，一个寄存器放8个通道的中断位
    // 因为我这里目前只有H3，所以我也只考虑H3的情况（12个通道，分两组，前一组8个，后一组4个）
    if(pchan->index < 8) {
        // 第一组
        reg_value = ioread32(dma_dev->base_addr + DMA_IRQ_EN_REG0_OFFSET);
        // 如果是0号通道，那就是0到2一共三个位置为0（第四个位保留，永远为0）
        // 三个位置全部为1的时候是7（7的二进制是111）
        // 如果是1号通道，那就是4到6一共是三个位置，那就是7 << 4 * 1，以此类推

        reg_value &= ~(7 << (4 * pchan->index));  // 位与按位取反，清除对应位的值
        reg_value |= (vchan->irq_type << (4 *  pchan->index));  // 设置指定值


        iowrite32(reg_value, dma_dev->base_addr + DMA_IRQ_EN_REG0_OFFSET);
    } else {
        // 第二组
        reg_value = ioread32(dma_dev->base_addr + DMA_IRQ_EN_REG1_OFFSET);

        reg_value &= ~(7 << (4 * (pchan->index - 8)));  // 位与按位取反，清除对应位的值
        reg_value |= (vchan->irq_type << (4 *  (pchan->index - 8)));    // 设置指定值

        iowrite32(reg_value, dma_dev->base_addr + DMA_IRQ_EN_REG1_OFFSET);
    }

    // 把对应的开始地址写到DMA通道开始地址寄存器
    iowrite32(pchan->todo->physical_addr,dma_dev->base_addr + DMA_DESC_ADDR_REG_OFFSET(pchan->index));
    // 开启传输
    iowrite32(1, dma_dev->base_addr + DMA_EN_REG_OFFSET(pchan->index));

    spin_unlock_irq(&vchan->vc.lock);

    return 0;  // 成功启动传输
}

/*
 * DMA传输停止函数
 * 功能: 停止DMA传输操作。
 */
static void hc_dma_stop_transfer(DMA_DEV_Info *dma_dev, DMA_Virtual_Channel_Info *vchan) {
    // 这个DMA不用停止，硬件描述符到头自动停止，因此保持原样
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

/*
 * 准备 XOR 操作的 DMA 描述符
 * 功能：为 XOR 操作分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_xor(
        struct dma_chan *chan, dma_addr_t dst, dma_addr_t *src,
        unsigned int src_cnt, size_t len, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @dst: 目标地址，XOR 结果将写入该地址
    // @src: 源地址数组，包含执行 XOR 操作的多个源数据块
    // @src_cnt: 源地址的数量，表示 src 数组中有多少数据块
    // @len: 数据块长度，表示每个源地址中的数据块长度
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备 XOR 验证操作的 DMA 描述符
 * 功能：为 XOR 校验操作分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_xor_val(
        struct dma_chan *chan, dma_addr_t *src, unsigned int src_cnt,
        size_t len, enum sum_check_flags *result, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @src: 源地址数组，包含用于验证的数据块
    // @src_cnt: 源地址的数量，表示 src 数组中有多少数据块
    // @len: 数据块长度，表示每个源地址中的数据块长度
    // @result: 验证结果存储的位置
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备 PQ 操作的 DMA 描述符
 * 功能：为 PQ 操作分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_pq(
        struct dma_chan *chan, dma_addr_t *dst, dma_addr_t *src,
        unsigned int src_cnt, const unsigned char *scf, size_t len, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @dst: 目标地址数组，PQ 结果将写入这些地址
    // @src: 源地址数组，包含 PQ 操作的多个源数据块
    // @src_cnt: 源地址的数量，表示 src 数组中有多少数据块
    // @scf: 源系数数组，用于在 PQ 操作中调整权重
    // @len: 数据块长度，表示每个源地址中的数据块长度
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备 PQ 校验和验证操作的 DMA 描述符
 * 功能：为 PQ 校验操作分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_pq_val(
        struct dma_chan *chan, dma_addr_t *pq, dma_addr_t *src,
        unsigned int src_cnt, const unsigned char *scf, size_t len,
        enum sum_check_flags *pqres, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @pq: 校验数据块地址
    // @src: 源地址数组，包含执行 PQ 验证的多个数据块
    // @src_cnt: 源地址的数量，表示 src 数组中有多少数据块
    // @scf: 源系数数组，用于 PQ 操作
    // @len: 数据块长度，表示每个源地址中的数据块长度
    // @pqres: 验证结果存储位置
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备执行内存填充操作的 DMA 描述符
 * 功能：为内存填充操作分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memset(
        struct dma_chan *chan, dma_addr_t dest, int value, size_t len, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @dest: 目标内存地址，填充数据将写入该地址
    // @value: 填充值，将用于填充指定的内存区域
    // @len: 内存填充的长度
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备 scatter-gather 形式的内存填充操作的 DMA 描述符
 * 功能：为 scatter-gather 形式的内存填充操作分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_memset_sg(
        struct dma_chan *chan, struct scatterlist *sg, unsigned int nents,
        int value, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @sg: scatter-gather 列表，包含多个分散的内存块
    // @nents: scatter-gather 列表中的条目数量
    // @value: 填充值，将用于填充内存块
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备传输完成时触发的中断的 DMA 描述符
 * 功能：为传输完成的中断分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_interrupt(
        struct dma_chan *chan, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备 scatter-gather 传输操作的 DMA 描述符
 * 功能：为 scatter-gather 传输分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_sg(
        struct dma_chan *chan, struct scatterlist *dst_sg, unsigned int dst_nents,
        struct scatterlist *src_sg, unsigned int src_nents, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @dst_sg: 目标 scatter-gather 列表，包含写入目标的多个分散数据块
    // @dst_nents: 目标 scatter-gather 列表中的条目数量
    // @src_sg: 源 scatter-gather 列表，包含源数据的多个分散数据块
    // @src_nents: 源 scatter-gather 列表中的条目数量
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备交错传输操作的 DMA 描述符
 * 功能：为交错传输分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_interleaved_dma(
        struct dma_chan *chan, struct dma_interleaved_template *xt, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @xt: 交错传输模板，包含交错数据传输的相关信息
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * 准备执行立即数据传输的 DMA 描述符
 * 功能：为立即数据传输分配 DMA 描述符
 */
static struct dma_async_tx_descriptor *hc_dma_prep_dma_imm_data(
        struct dma_chan *chan, dma_addr_t dst, uint64_t data, unsigned long flags) {

    // @chan: DMA 通道，用于指定在哪个通道上执行操作
    // @dst: 目标地址，将写入立即数据
    // @data: 立即数据，大小为 8 字节
    // @flags: 描述符标志，可能包括 DMA 操作的优先级或模式等

    /* 函数实现逻辑 */
    return NULL;
}

/*
 * DMA同步函数
 * 功能：同步 DMA 终止操作，确保传输任务安全结束
 */
static void hc_dma_synchronize(struct dma_chan *chan) {
    // @chan: DMA 通道，用于指定在哪个通道上执行操作

    /* 函数实现逻辑 */
}
