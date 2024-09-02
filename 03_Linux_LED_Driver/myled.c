//
// Created by huangcheng on 2024/9/2.
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h>               // 包含ioremap和iowrite/ioread函数

// 编写所涉及的函数
static int dev_open(struct inode *inodep, struct file *filep);
static int dev_release(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *filep, char *user_buffer, size_t len, loff_t *offset);
static ssize_t dev_write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset);

// 文件操作符，通过这个，就可以像操作文件一样操作设备（把设备抽象成文件就是这么抽象的）
static struct file_operations fops = {
        .open = dev_open,
        .release = dev_release,
        .read = dev_read,
        .write = dev_write,
};

// 设备驱动模块初始化函数
static int __init dev_module_init(void);
// 设备驱动模块退出函数
static void __exit dev_module_exit(void);

// 指定模块初始化函数，这样内核加载模块时可以自动初始化设备模块
module_init(dev_module_init);
// 指定模块退出函数，这样内核卸载模块时可以自动退出设备模块
module_exit(dev_module_exit);

// 设备驱动相关信息，这里写了就可以查询文件信息得到
MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng");
MODULE_DESCRIPTION("A simple led driver for OrangePi One");
MODULE_VERSION("1.0");

// 设备名
#define DEVICE_NAME "myled"
// 设备类名（在/dev/文件夹中创建设备文件需要，这是为了归类设备的文件夹）
#define CLASS_NAME "leddev"

// 设备号，向内核注册驱动的时候获得的一个唯一ID，通过设备号可以找到这个设备
static int majorNumber;
// 设备类（用来将设备归类，在对应的/dev/ClassName文件夹下可以看到相应的设备）
static struct class* ledClass = NULL;
// 设备结构体（表示内核中的一个实际的设备，因为一个设备可能还有很多子设备，同一个设备号可能表示多个设备，设备结构体才是一一对应的）
static struct device* ledDevice = NULL;

// 实现设备功能所需的全局变量、数据结构等

// 互斥锁，保护设备资源同时只能被一个用户访问
static struct mutex dev_mutex;
// 红色led灯的地址
void __iomem *red_led_base_addr;
// 绿色led灯的地址
void __iomem *green_led_base_addr;
// 两个灯的物理地址
#define RED_LED_BASE 0x01c20800     // 对应A组15引脚
#define GREEN_LED_BASE 0x01f02c00   // 对应L组10引脚
// 两个灯的当前状态
static int red_led_status = 0;
static int green_led_status = 0;
// 寄存器偏移量
#define GPIO_CFG_OFFSET 0x00        // 配置寄存器
#define GPIO_DATA_OFFSET 0x10       // 数据寄存器

static int __init dev_module_init(void) {
    // 这个函数的主要作用就是向内核注册设备，，并且初始化设备所需的资源

    // 向内核注册字符设备（Linux分为字符设备和块设备两种），卸载设备的时候需要设备号
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "Registered correctly with major number %d\n", majorNumber);

    // 注册设备类
    ledClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ledClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ledClass);
    }
    printk(KERN_INFO "Device class registered correctly\n");

    // 注册设备驱动
    ledDevice = device_create(ledClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ledDevice)) {
        class_destroy(ledClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ledDevice);
    }
    printk(KERN_INFO "Device class created correctly\n");

    // 初始化设备所需资源（这里是内存映射、led状态和互斥锁）

    mutex_init(&dev_mutex);

    // 使用ioremap映射数据手册上对应的物理地址，范围应该选取尽量小
    red_led_base_addr = ioremap(RED_LED_BASE, 0x20);
    green_led_base_addr = ioremap(GREEN_LED_BASE, 0x20);
    // 读取状态
    red_led_status = ioread32(red_led_base_addr + GPIO_DATA_OFFSET) & (1 << 15);
    green_led_status = ioread32(green_led_base_addr + GPIO_DATA_OFFSET)  & (1 << 10);

    // 把红色LED对应的GPIO设置为输出模式
    unsigned int cfg_value = ioread32(red_led_base_addr + GPIO_CFG_OFFSET);         // 设置红色LED对应的GPIO引脚
    cfg_value &= ~(0xf << 28);                                                      // 更新写入值
    iowrite32(cfg_value, red_led_base_addr + GPIO_CFG_OFFSET);                      // 清除第15引脚配置
    cfg_value |= (1 << 28);                                                         // 更新写入值
    iowrite32(cfg_value, red_led_base_addr + GPIO_CFG_OFFSET);                      // 设置这个引脚为输出模式

    // 把绿色LED对应的GPIO设置为输出模式
    cfg_value = ioread32(green_led_base_addr + GPIO_CFG_OFFSET);                    // 设置绿色LED对应的GPIO引脚
    cfg_value &= ~(0xf << 20);                                                      // 更新写入值
    iowrite32(cfg_value, green_led_base_addr + GPIO_CFG_OFFSET);                    // 清除第10个引脚配置
    cfg_value |= (1 << 20);                                                         // 更新写入值
    iowrite32(cfg_value, green_led_base_addr + GPIO_CFG_OFFSET);                    // 设置这个引脚为输出模式

    printk(KERN_INFO "Device initialized\n");

    return 0;
}

static void __exit dev_module_exit(void) {
    // 这个函数的主要作用是销毁资源，并从内核中删除设备注册信息

    // 销毁设备类，这样就从/dev/文件夹中删除了
    device_destroy(ledClass, MKDEV(majorNumber, 0));
    class_unregister(ledClass);
    class_destroy(ledClass);
    // 删除设备注册信息
    unregister_chrdev(majorNumber, DEVICE_NAME);

    // 销毁资源（这里是互斥锁和内存映射、状态）
    mutex_destroy(&dev_mutex);
    // 释放映射的内存，状态归0
    if (green_led_base_addr) {
        iounmap(green_led_base_addr);
        green_led_status = 0;
    }

    printk(KERN_INFO "Device unregistered\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    // 这里打开操作直接打印结果就行，能执行肯定已经打开了
    printk(KERN_INFO "myled: Device has been opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    // 这个虚拟设备释放操作也不需要干什么，直接打印结果就是了
    printk(KERN_INFO "myled: Device successfully closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *user_buffer, size_t len, loff_t *offset) {

    char status[64];    // 结果字符串
    int error_count = 0;
    int status_len = 0;

    // 只允许在文件偏移量为0时读取数据，防止cat命令一直读取没完没了
    if (*offset > 0) {
        return 0;       // 返回0表示文件已读取完毕
    }

    if (mutex_lock_interruptible(&dev_mutex)) {
        return -ERESTARTSYS;
    }

    // 更新状态，并将当前状态打印到终端
    red_led_status = ioread32(red_led_base_addr + GPIO_DATA_OFFSET) & (1 << 15);
    green_led_status = ioread32(green_led_base_addr + GPIO_DATA_OFFSET)  & (1 << 10);

    // 拼接结果字符串
    snprintf(status, sizeof(status), "Green LED status: %s, Red LED status: %s\n",
             green_led_status ? "ON" : "OFF",
             red_led_status ? "ON" : "OFF");

    status_len = strnlen(status, sizeof(status));  // 确保长度安全

    mutex_unlock(&dev_mutex);

    // 将结果字符串复制到用户空间
    error_count = copy_to_user(user_buffer, status, status_len);

    if (error_count == 0) {
        *offset = status_len;  // 更新偏移量，表示数据已读取
        return status_len;      // 返回读取的字节数
    } else {
        printk(KERN_INFO "myled: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset) {
    char input;

    if (len > 0) {
        // 输入0或者1，输入1则改变当前两个led状态（各自独立），输入0则不变
        if (copy_from_user(&input, user_buffer, 1) != 0) {
            return -EFAULT;
        }

        if (mutex_lock_interruptible(&dev_mutex)) {
            return -ERESTARTSYS;
        }

        switch (input) {
            case '1':
                // 切换红色LED的状态
                if (red_led_status) {
                    iowrite32(ioread32(red_led_base_addr + GPIO_DATA_OFFSET) & ~(1 << 15), red_led_base_addr + GPIO_DATA_OFFSET);
                    red_led_status = 0;
                    printk(KERN_INFO "myled: Red LED turned OFF\n");
                } else {
                    iowrite32(ioread32(red_led_base_addr + GPIO_DATA_OFFSET) | (1 << 15), red_led_base_addr + GPIO_DATA_OFFSET);
                    red_led_status = 1;
                    printk(KERN_INFO "myled: Red LED turned ON\n");
                }

                // 切换绿色LED的状态
                if (green_led_status) {
                    iowrite32(ioread32(green_led_base_addr + GPIO_DATA_OFFSET) & ~(1 << 10), green_led_base_addr + GPIO_DATA_OFFSET);
                    green_led_status = 0;
                    printk(KERN_INFO "myled: Green LED turned OFF\n");
                } else {
                    iowrite32(ioread32(green_led_base_addr + GPIO_DATA_OFFSET) | (1 << 10), green_led_base_addr + GPIO_DATA_OFFSET);
                    green_led_status = 1;
                    printk(KERN_INFO "myled: Green LED turned ON\n");
                }
                break;

            default:
                printk(KERN_INFO "myled: Invalid input\n");
                break;
        }

        mutex_unlock(&dev_mutex);
    }
    return len;
}
