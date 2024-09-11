#include <linux/module.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/scatterlist.h>
#include <linux/string.h>
#include <linux/io.h>

#define DMA_BUFFER_SIZE 1024

static dma_addr_t dma_src, dma_dst;
static char *src_buf, *dst_buf, *device_mem;  // 模拟设备的内存
static struct dma_chan *dma_chan;
static struct completion dma_complete;

static void dma_callback(void *completion)
{
    complete(completion);
    pr_info("DMA transfer completed\n");
}

static int verify_data(const char *src, const char *dst, size_t size)
{
    if (memcmp(src, dst, size) == 0) {
        pr_info("Data verification successful\n");
        return 0;
    } else {
        pr_err("Data verification failed\n");
        return -1;
    }
}

static int dma_test(enum dma_transfer_direction direction)
{
    struct dma_device *dma_dev;
    struct dma_async_tx_descriptor *tx;
    dma_cap_mask_t mask;
    struct dma_slave_config slave_config;
    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    int ret;
    dma_addr_t device_phys_addr;

    pr_info("Cyclic DMA test started\n");

    // Step 1: 设置DMA引擎能力掩码
    dma_cap_zero(mask);
    dma_cap_set(DMA_SLAVE, mask);

    // Step 2: 请求DMA通道
    dma_chan = dma_request_chan_by_mask(&mask);
    if (IS_ERR(dma_chan)) {
        pr_err("Failed to request DMA channel\n");
        return PTR_ERR(dma_chan);
    }

    dma_dev = dma_chan->device;

    // Step 3: 分配源和目标缓冲区，以及模拟设备的内存
    src_buf = kzalloc(DMA_BUFFER_SIZE, GFP_KERNEL);
    dst_buf = kzalloc(DMA_BUFFER_SIZE, GFP_KERNEL);
    device_mem = kzalloc(DMA_BUFFER_SIZE, GFP_KERNEL);  // 模拟设备的内存

    if (!src_buf || !dst_buf || !device_mem) {
        pr_err("Failed to allocate buffers\n");
        ret = -ENOMEM;
        goto err_free_buf;
    }

    // 初始化源缓冲区数据
    memset(src_buf, 0xAA, DMA_BUFFER_SIZE);  // 用特定的模式填充源缓冲区
    memset(device_mem, 0, DMA_BUFFER_SIZE);  // 初始化模拟设备内存
    memset(dst_buf, 0, DMA_BUFFER_SIZE);     // 初始化目标缓冲区

    // 获取设备模拟内存的物理地址
    device_phys_addr = virt_to_phys(device_mem);

    // Step 4: 配置 DMA slave 传输
    memset(&slave_config, 0, sizeof(slave_config));
    slave_config.direction = direction;
    slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES; // 根据实际配置
    slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES; // 根据实际配置
    slave_config.src_maxburst = 8; // 合理设置
    slave_config.dst_maxburst = 8; // 合理设置

    pr_info("Configuring DMA: src_maxburst=%d, dst_maxburst=%d, src_addr_width=%d, dst_addr_width=%d\n",
            slave_config.src_maxburst, slave_config.dst_maxburst,
            slave_config.src_addr_width, slave_config.dst_addr_width);

    ret = dmaengine_slave_config(dma_chan, &slave_config);
    if (ret) {
        pr_err("Failed to configure DMA slave\n");
        goto err_free_buf;
    }

    // Step 5: 设置DMA循环传输
    dma_src = dma_map_single(dma_dev->dev, src_buf, DMA_BUFFER_SIZE, DMA_TO_DEVICE);
    if (dma_mapping_error(dma_dev->dev, dma_src)) {
        pr_err("DMA mapping error\n");
        ret = -EIO;
        goto err_free_buf;
    }

    // Step 6: 准备循环DMA传输
    tx = dmaengine_prep_dma_cyclic(dma_chan, dma_src, DMA_BUFFER_SIZE, DMA_BUFFER_SIZE / 2, direction, flags);
    if (!tx) {
        pr_err("Failed to prepare cyclic DMA transfer\n");
        ret = -EIO;
        goto err_free_buf;
    }

    // Step 7: 设置DMA传输完成的回调函数
    init_completion(&dma_complete);
    tx->callback = dma_callback;
    tx->callback_param = &dma_complete;

    // Step 8: 提交并启动DMA传输
    dmaengine_submit(tx);
    dma_async_issue_pending(dma_chan);

    // Step 9: 等待DMA传输完成
    wait_for_completion(&dma_complete);

    pr_info("Cyclic DMA transfer completed\n");

    // Step 10: 数据传输后的完整性校验
    if (direction == DMA_MEM_TO_DEV) {
        pr_info("Verifying data from memory to device\n");
        ret = verify_data(src_buf, device_mem, DMA_BUFFER_SIZE);  // 校验内存到设备
    } else if (direction == DMA_DEV_TO_MEM) {
        pr_info("Verifying data from device to memory\n");
        ret = verify_data(device_mem, dst_buf, DMA_BUFFER_SIZE);  // 校验设备到内存
    }

err_free_buf:
    dma_unmap_single(dma_dev->dev, dma_src, DMA_BUFFER_SIZE, DMA_TO_DEVICE);
    kfree(src_buf);
    kfree(dst_buf);
    kfree(device_mem);
    dma_release_channel(dma_chan);

    return ret;
}

static int __init dma_test_init(void)
{
    pr_info("Initializing Cyclic DMA test module\n");

    // 只能测试从模拟设备到内存

    // 测试从设备到内存的循环DMA传输
    if (dma_test(DMA_DEV_TO_MEM) < 0) {
        pr_err("Device to memory Cyclic DMA test failed\n");
    }

    return 0;
}

static void __exit dma_test_exit(void)
{
    pr_info("Exiting Cyclic DMA test module\n");
}

module_init(dma_test_init);
module_exit(dma_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng");
MODULE_DESCRIPTION("Cyclic DMA test module with data verification and physical address handling");
