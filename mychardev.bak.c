#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/of_dma.h>
#include <linux/completion.h>

#define DEVICE_NAME "mychardev"
#define MAJOR_NUM 240  // Choose static major number >= 240

// static char message[128] = "Hello from kernel!\n";
// static struct dma_chan *dma_chan;
// static struct completion dma_done;
// 
// static int my_open(struct inode *inode, struct file *file)
// {
//     pr_info("mychardev: device opened\n");
//     return 0;
// }
// 
// static int my_release(struct inode *inode, struct file *file)
// {
//     pr_info("mychardev: device closed\n");
//     return 0;
// }
// 
// static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
// {
//     return simple_read_from_buffer(buf, len, offset, message, strlen(message));
// }
// 
// static struct file_operations fops = {
//     .owner = THIS_MODULE,
//     .open = my_open,
//     .release = my_release,
//     .read = my_read,
// };
// 
// static struct device *find_device_by_of_node(struct device_node *np)
// {
// 	return bus_find_device(&platform_bus_type, NULL, np,
// 	                       (void *)of_node_cmp);
// }
// static int __init mychardev_init(void)
// {
//     int ret;
//     dma_cap_mask_t mask;
//     struct dma_async_tx_descriptor *tx;
//     dma_addr_t dma_dst;
//     void *cpu_dst;
//     size_t len = 4096;
// 
// 
//     struct device_node *np = of_find_node_by_path("/axi-dma-client");
//     if (!np) {
//     	pr_err("np not found");
// 	return -1; 
//     }
//     pr_info("Found node: %s\n", np->name);
// 
// 
//     struct platform_device *pdev;
//     pdev = of_find_device_by_node(np);
//     
//     if (!pdev) {
//         pr_err("Failed to create platform device for DMA client\n");
//         return -ENODEV;
//     }
//     struct dma_chan *chan = of_dma_request_slave_channel(np, "s2mm");
//     if (!dma_chan) {
//      	pr_err("no dma chan");
// 	return -1; 
// 
//     }
// 
// 
// 
// 
// 
//     ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
//     if (ret < 0) {
//         pr_err("mychardev: failed to register device\n");
//         return ret;
//     }
// 
//     pr_info("mychardev: registered with major %d\n", MAJOR_NUM);
//     return 0;
// }
// 
// static void __exit mychardev_exit(void)
// {
//     unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
//     pr_info("mychardev: unregistered\n");
// }
// 
// module_init(mychardev_init);
// module_exit(mychardev_exit);
// 
// MODULE_LICENSE("GPL");
// MODULE_AUTHOR("Anton");
// MODULE_DESCRIPTION("Minimal char device driver");
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

static int my_driver_probe(struct platform_device *pdev)
{
    struct dma_chan *chan;

    dev_info(&pdev->dev, "Probing my DMA client\n");
    struct device *dev = &pdev->dev;

    dev_dbg(dev, "Node name: %s\n", dev->of_node->full_name);

    chan = of_dma_request_slave_channel(dev->of_node, "s2mm");
    if (!chan){
    	dev_err(dev, "Failed to request DMA channel\n");
    }
    if (IS_ERR(chan)) {
        pr_err("Failed to request DMA channel: %ld\n", PTR_ERR(chan));
        return PTR_ERR(chan);
    }
    if (!chan) {
        dev_err(&pdev->dev, "Failed to get DMA channel\n");
        return -ENODEV;
    }

    dev_info(&pdev->dev, "Got DMA channel: %s\n", dma_chan_name(chan));

    // Save chan for later use
    // dma_release_channel(chan); // Call this when done

    return 0;
}

static int my_driver_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Driver removed\n");
    return 0;
}

static const struct of_device_id my_driver_of_match[] = {
    { .compatible = "xlnx,axi-dma-client" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_driver_of_match);

static struct platform_driver my_driver = {
    .probe = my_driver_probe,
    .remove = my_driver_remove,
    .driver = {
        .name = "my_dma_client_driver",
        .of_match_table = my_driver_of_match,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("you");
MODULE_DESCRIPTION("AXI DMA test driver");
