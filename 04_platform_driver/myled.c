//
// Created by huangcheng on 2024/9/3.
//

#include "myled.h"

// 匹配设备树中的`compatible`字段，通过这个，找到对应的结点
static struct of_device_id myled_of_match[] = {
        { .compatible = "huangcheng,myled", },
        { },
};
MODULE_DEVICE_TABLE(of, myled_of_match);

// 平台驱动结构体，通过of_match_table来匹配到对应的结点，进而载入平台设备
static struct platform_driver myled_driver = {
        .probe = led_probe,
        .remove = led_remove,
        .driver = {
                .name = DEVICE_NAME,
                .of_match_table = myled_of_match,
                .owner = THIS_MODULE,
        },
};

module_platform_driver(myled_driver);

// 文件操作符，通过这个，就可以像操作文件一样操作设备（把设备抽象成文件就是这么抽象的）
static struct file_operations fops = {
        .open = dev_open,
        .release = dev_release,
        .read = dev_read,
        .write = dev_write,
};

// 设备号，向内核注册驱动的时候获得的一个唯一ID，通过设备号可以找到这个设备
static int majorNumber;
// 设备类（用来将设备归类，在对应的/dev/ClassName文件夹下可以看到相应的设备）
static struct class* devClass = NULL;

// 全局设备数据结构指针（每个设备自己单独的资源）
static struct led_dev *red_led_device_data;
static struct led_dev *green_led_device_data;

// 寄存器偏移量
#define GPIO_CFG_OFFSET 0x00        // 配置寄存器
#define GPIO_DATA_OFFSET 0x10       // 数据寄存器

static int led_probe(struct platform_device *pdev) {
    // pdev 是 myled_driver结构体给出的参数，匹配后就取到了

    struct resource *res;
    dev_t dev_no;
    int err;
    unsigned int cfg_value;

    // 分配内存给红色LED设备数据结构，用devm_kzalloc的话，意外退出会自动被内核回收所有资源，就不用自行回收了
    red_led_device_data = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);
    if (!red_led_device_data) {
        dev_err(&pdev->dev, "Failed to allocate memory for red_led_device_data\n");
        return -ENOMEM;
    }

    // 分配内存给绿色LED设备数据结构
    green_led_device_data = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);
    if (!green_led_device_data) {
        dev_err(&pdev->dev, "Failed to allocate memory for green_led_device_data\n");
        return -ENOMEM;
    }

    // 从设备树中获取红色LED的资源，通过资源的名字就能获取，红色，对应的资源就是<0x01c20800, 0x20>
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "red");
    if (!res) {
        dev_err(&pdev->dev, "Failed to get red LED resource\n");
        return -ENODEV;
    }
    // 理论上来说应该这么写才对，但是devm_ioremap_resource会申请映射，会碰上Linux中的驱动已经映射的问题，所以只能用ioremap强行占据
    // red_led_device_data->led_base_addr = devm_ioremap_resource(&pdev->dev, res);
    red_led_device_data->led_base_addr = ioremap(res->start, resource_size(res));

    // 如果没获取到资源就直接退出返回顺便报个错
    if (IS_ERR(red_led_device_data->led_base_addr)) {
        dev_err(&pdev->dev, "Failed to map red LED registers\n");
        return PTR_ERR(red_led_device_data->led_base_addr);
    }

    // 从设备树中获取绿色LED的资源
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "green");
    if (!res) {
        dev_err(&pdev->dev, "Failed to get green LED resource\n");
        return -ENODEV;
    }
    // 同red_led_device_data
    // green_led_device_data->led_base_addr = devm_ioremap_resource(&pdev->dev, res);
    green_led_device_data->led_base_addr = ioremap(res->start, resource_size(res));

    if (IS_ERR(green_led_device_data->led_base_addr)) {
        dev_err(&pdev->dev, "Failed to map green LED registers\n");
        return PTR_ERR(green_led_device_data->led_base_addr);
    }

    // 注册字符设备主设备，把文件操作符传过去，这样就能通过文件操作符像操作普通文件一样操作设备了
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        dev_err(&pdev->dev, "Failed to register a major number\n");
        return majorNumber;
    }

    // 创建设备类
    devClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(devClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        dev_err(&pdev->dev, "Failed to create device class\n");
        return PTR_ERR(devClass);
    }

    // 创建设备文件节点（红色LED），这里就分主从设备了，红色LED是从设备0
    dev_no = MKDEV(majorNumber, 0);
    cdev_init(&red_led_device_data->cdev, &fops);
    red_led_device_data->cdev.owner = THIS_MODULE;
    err = cdev_add(&red_led_device_data->cdev, dev_no, 1);
    if (err) {
        class_destroy(devClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        dev_err(&pdev->dev, "Failed to add red LED cdev\n");
        return err;
    }
    device_create(devClass, NULL, dev_no, NULL, "myled_red");

    // 创建设备文件节点（绿色LED），绿色LED是从设备1
    dev_no = MKDEV(majorNumber, 1);
    cdev_init(&green_led_device_data->cdev, &fops);
    green_led_device_data->cdev.owner = THIS_MODULE;
    err = cdev_add(&green_led_device_data->cdev, dev_no, 1);
    if (err) {
        device_destroy(devClass, MKDEV(majorNumber, 0));
        cdev_del(&red_led_device_data->cdev);
        class_destroy(devClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        dev_err(&pdev->dev, "Failed to add green LED cdev\n");
        return err;
    }
    device_create(devClass, NULL, dev_no, NULL, "myled_green");

    // 最后初始化所有资源

    // 初始化互斥锁
    mutex_init(&red_led_device_data->dev_mutex);
    mutex_init(&green_led_device_data->dev_mutex);

    // 把红色LED对应的GPIO设置为输出模式
    cfg_value = ioread32(red_led_device_data->led_base_addr + GPIO_CFG_OFFSET);                 // 设置红色LED对应的GPIO引脚
    cfg_value &= ~(0xf << 28);                                                                  // 更新写入值
    iowrite32(cfg_value, red_led_device_data->led_base_addr + GPIO_CFG_OFFSET);                 // 清除第15引脚配置
    cfg_value |= (1 << 28);                                                                     // 更新写入值
    iowrite32(cfg_value, red_led_device_data->led_base_addr + GPIO_CFG_OFFSET);                 // 设置这个引脚为输出模式

    // 把绿色LED对应的GPIO设置为输出模式
    cfg_value = ioread32(green_led_device_data->led_base_addr + GPIO_CFG_OFFSET);               // 设置绿色LED对应的GPIO引脚
    cfg_value &= ~(0xf << 20);                                                                  // 更新写入值
    iowrite32(cfg_value, green_led_device_data->led_base_addr + GPIO_CFG_OFFSET);               // 清除第10个引脚配置
    cfg_value |= (1 << 20);                                                                     // 更新写入值
    iowrite32(cfg_value, green_led_device_data->led_base_addr + GPIO_CFG_OFFSET);               // 设置这个引脚为输出模式

    printk(KERN_INFO "LED platform driver initialized successfully\n");
    return 0;
}

static int led_remove(struct platform_device *pdev) {
    // 删除红色LED的设备节点和字符设备
    device_destroy(devClass, MKDEV(majorNumber, 0));
    cdev_del(&red_led_device_data->cdev);

    // 删除绿色LED的设备节点和字符设备
    device_destroy(devClass, MKDEV(majorNumber, 1));
    cdev_del(&green_led_device_data->cdev);

    // 销毁设备类
    class_destroy(devClass);

    // 注销字符设备
    unregister_chrdev(majorNumber, DEVICE_NAME);

    // 销毁互斥锁
    mutex_destroy(&red_led_device_data->dev_mutex);
    mutex_destroy(&green_led_device_data->dev_mutex);

    // 因为资源的内存是用devm_kzalloc申请的
    // 在退出的时候，内存会被内核自动回收，所以不用自己回收了

    printk(KERN_INFO "LED platform driver removed successfully\n");
    return 0;
}

static int dev_open(struct inode *inodep, struct file *filep) {
    struct led_dev *dev;

    dev = container_of(inodep->i_cdev, struct led_dev, cdev);
    filep->private_data = dev;

    printk(KERN_INFO "myled: Device has been opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "myled: Device successfully closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *user_buffer, size_t len, loff_t *offset) {
    struct led_dev *dev = filep->private_data;  // 获取当前设备的数据结构
    char status[16];  // 保存状态信息的字符串
    int error_count = 0;
    int status_len;

    // 只允许在文件偏移量为0时读取数据，防止重复读取
    if (*offset > 0) {
        return 0;  // EOF
    }

    if (mutex_lock_interruptible(&dev->dev_mutex)) {
        return -ERESTARTSYS;
    }

    // 读取LED的当前状态
    if (dev == red_led_device_data) {
        dev->led_status = (ioread32(dev->led_base_addr + GPIO_DATA_OFFSET) & (1 << 15)) ? 1 : 0;
    } else if (dev == green_led_device_data) {
        dev->led_status = (ioread32(dev->led_base_addr + GPIO_DATA_OFFSET) & (1 << 10)) ? 1 : 0;
    }

    // 将状态转换为字符串，表示 LED 是 ON 还是 OFF
    snprintf(status, sizeof(status), "LED is %s\n", dev->led_status ? "ON" : "OFF");
    status_len = strlen(status);

    // 将状态字符串复制到用户空间
    error_count = copy_to_user(user_buffer, status, status_len);

    mutex_unlock(&dev->dev_mutex);

    if (error_count == 0) {
        *offset = status_len;
        return status_len;
    } else {
        printk(KERN_INFO "myled: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset) {
    struct led_dev *dev = filep->private_data;  // 获取当前设备的数据结构
    char input;

    if (len > 0) {
        // 从用户空间复制一个字符数据到内核空间
        if (copy_from_user(&input, user_buffer, 1) != 0) {
            return -EFAULT;
        }

        if (mutex_lock_interruptible(&dev->dev_mutex)) {
            return -ERESTARTSYS;
        }

        // 根据输入决定是否切换LED状态
        switch (input) {
            case '1':
                // 打开LED
                if (dev == red_led_device_data) {
                    iowrite32(ioread32(dev->led_base_addr + GPIO_DATA_OFFSET) | (1 << 15), dev->led_base_addr + GPIO_DATA_OFFSET);
                    dev->led_status = 1;
                    printk(KERN_INFO "myled: Red LED turned ON\n");
                } else if (dev == green_led_device_data) {
                    iowrite32(ioread32(dev->led_base_addr + GPIO_DATA_OFFSET) | (1 << 10), dev->led_base_addr + GPIO_DATA_OFFSET);
                    dev->led_status = 1;
                    printk(KERN_INFO "myled: Green LED turned ON\n");
                }
                break;

            case '0':
                // 关闭LED
                if (dev == red_led_device_data) {
                    iowrite32(ioread32(dev->led_base_addr + GPIO_DATA_OFFSET) & ~(1 << 15), dev->led_base_addr + GPIO_DATA_OFFSET);
                    dev->led_status = 0;
                    printk(KERN_INFO "myled: Red LED turned OFF\n");
                } else if (dev == green_led_device_data) {
                    iowrite32(ioread32(dev->led_base_addr + GPIO_DATA_OFFSET) & ~(1 << 10), dev->led_base_addr + GPIO_DATA_OFFSET);
                    dev->led_status = 0;
                    printk(KERN_INFO "myled: Green LED turned OFF\n");
                }
                break;

            default:
                printk(KERN_INFO "myled: Invalid input\n");
                break;
        }

        mutex_unlock(&dev->dev_mutex);
    }
    return len;
}
