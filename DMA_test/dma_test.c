//
// Created by huangcheng on 2024/9/6.
//

#include <linux/module.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/completion.h>

#define DMA_BUFFER_SIZE 1024

static dma_addr_t dma_src, dma_dst;
static char *src_buf, *dst_buf;
static struct dma_chan *dma_chan;
static struct completion dma_complete;

static void dma_callback(void *completion)
{
    complete(completion);
    pr_info("DMA transfer completed\n");
}

static int dma_test(void)
{
    struct dma_device *dma_dev;
    struct dma_async_tx_descriptor *tx;
    dma_cap_mask_t mask;
    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    int ret;

    pr_info("DMA test started\n");

    // Step 1: 设置DMA引擎能力掩码，表示支持内存到内存的DMA传输
    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);

    // Step 2: 请求DMA通道
    dma_chan = dma_request_chan_by_mask(&mask);
    if (IS_ERR(dma_chan)) {
        pr_err("Failed to request DMA channel\n");
        return PTR_ERR(dma_chan);
    }

    dma_dev = dma_chan->device;

    // Step 3: 分配源和目标缓冲区
    src_buf = kzalloc(DMA_BUFFER_SIZE, GFP_KERNEL);
    dst_buf = kzalloc(DMA_BUFFER_SIZE, GFP_KERNEL);
    if (!src_buf || !dst_buf) {
        pr_err("Failed to allocate buffers\n");
        ret = -ENOMEM;
        goto err_free_buf;
    }

    // Step 4: 将源和目标缓冲区映射到DMA地址空间
    dma_src = dma_map_single(dma_dev->dev, src_buf, DMA_BUFFER_SIZE, DMA_TO_DEVICE);
    dma_dst = dma_map_single(dma_dev->dev, dst_buf, DMA_BUFFER_SIZE, DMA_FROM_DEVICE);

    if (dma_mapping_error(dma_dev->dev, dma_src) || dma_mapping_error(dma_dev->dev, dma_dst)) {
        pr_err("DMA mapping error\n");
        ret = -EIO;
        goto err_unmap;
    }

    // Step 5: 准备DMA传输
    tx = dma_dev->device_prep_dma_memcpy(dma_chan, dma_dst, dma_src, DMA_BUFFER_SIZE, flags);
    if (!tx) {
        pr_err("Failed to prepare DMA memcpy\n");
        ret = -EIO;
        goto err_unmap;
    }

    // Step 6: 设置DMA传输完成的回调函数
    init_completion(&dma_complete);
    tx->callback = dma_callback;
    tx->callback_param = &dma_complete;

    // Step 7: 提交并启动DMA传输
    dmaengine_submit(tx);
    dma_async_issue_pending(dma_chan);

    // Step 8: 等待DMA传输完成
    wait_for_completion(&dma_complete);

    pr_info("DMA test finished successfully\n");
    ret = 0;

    err_unmap:
    dma_unmap_single(dma_dev->dev, dma_src, DMA_BUFFER_SIZE, DMA_TO_DEVICE);
    dma_unmap_single(dma_dev->dev, dma_dst, DMA_BUFFER_SIZE, DMA_FROM_DEVICE);

    err_free_buf:
    kfree(src_buf);
    kfree(dst_buf);
    dma_release_channel(dma_chan);

    return ret;
}

static int __init dma_test_init(void)
{
    pr_info("Initializing DMA test module\n");
    return dma_test();
}

static void __exit dma_test_exit(void)
{
    pr_info("Exiting DMA test module\n");
}

module_init(dma_test_init);
module_exit(dma_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng");
MODULE_DESCRIPTION("Basic DMA test module");
