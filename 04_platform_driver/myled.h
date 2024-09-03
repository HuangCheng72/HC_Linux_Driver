//
// Created by huangcheng on 2024/9/3.
//

#ifndef HC_LINUX_DRIVER_MYLED_H
#define HC_LINUX_DRIVER_MYLED_H

// 平台驱动的开发框架

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

// 如果有多个相同的设备，就可以复用这一数据结构
// 而且，把所需的资源封装在结构体里面，而不是搞成多个全局变量，会更加安全
struct led_dev {
    struct cdev cdev;
    void __iomem *led_base_addr;    // led的地址
    int led_status;                 // led的状态，0是灭，1是亮
    struct mutex dev_mutex;         // 互斥锁，保证同时只有一个执行流可以操作设备
};

// 设备名
#define DEVICE_NAME "myled"
// 设备类名（在/dev/文件夹中创建设备文件需要，这是为了归类设备的文件夹）
#define CLASS_NAME "leddev"

// 平台驱动初始化函数
static int led_probe(struct platform_device *pdev);
// 平台驱动退出函数
static int led_remove(struct platform_device *pdev);

// 作为字符设备的情况下需要实现的函数
static int dev_open(struct inode *inodep, struct file *filep);
static int dev_release(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *filep, char *user_buffer, size_t len, loff_t *offset);
static ssize_t dev_write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset);

// 设备驱动相关信息，这里写了就可以查询文件信息得到
MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng");
MODULE_DESCRIPTION("A platform led driver for OrangePi One");
MODULE_VERSION("1.0");

#endif //HC_LINUX_DRIVER_MYLED_H
