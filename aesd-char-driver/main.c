/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h>   // kmalloc / kfree
#include <linux/uaccess.h> // copy_to_user / copy_from_user
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Muthuu SVS"); 
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev;
    PDEBUG("open");
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

static ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *entry;
    size_t entry_offset;
    ssize_t retval = 0;

    PDEBUG("read %zu bytes with offset %lld", count, *f_pos);

    if (mutex_lock_interruptible(&dev->buf_mutex))
        return -ERESTARTSYS;

    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer,
                                                             *f_pos,
                                                             &entry_offset);
    if (!entry)
        goto out;

    retval = (ssize_t)min(count, entry->size - entry_offset);

    if (copy_to_user(buf, entry->buffptr + entry_offset, retval)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += retval;

out:
    mutex_unlock(&dev->buf_mutex);
    return retval;
}

static ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    struct aesd_dev *dev = filp->private_data;
    const char *evicted;
    char *new_buf;
    char *kbuf;

    PDEBUG("write %zu bytes with offset %lld", count, *f_pos);

    /* Copy incoming data to a local buffer outside the lock — no shared state */
    kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        return -EFAULT;
    }

    if (mutex_lock_interruptible(&dev->buf_mutex)) {
        kfree(kbuf);
        return -ERESTARTSYS;
    }

    /* Grow working_entry to append the new bytes */
    new_buf = krealloc(dev->working_entry.buffptr,
                       dev->working_entry.size + count,
                       GFP_KERNEL);
    if (!new_buf) {
        kfree(kbuf);
        mutex_unlock(&dev->buf_mutex);
        return -ENOMEM;
    }

    memcpy(new_buf + dev->working_entry.size, kbuf, count);
    kfree(kbuf);

    dev->working_entry.buffptr  = new_buf;
    dev->working_entry.size    += count;

    /* Only commit to the circular buffer once a \n terminator is present */
    if (memchr(dev->working_entry.buffptr, '\n', dev->working_entry.size)) {
        evicted = aesd_circular_buffer_add_entry(&dev->buffer, &dev->working_entry);
        if (evicted)
            kfree((void *)evicted);

        /* circular buffer now owns the buffptr — reset working entry */
        dev->working_entry.buffptr = NULL;
        dev->working_entry.size    = 0;
    }

    mutex_unlock(&dev->buf_mutex);
    return (ssize_t)count;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device, 0, sizeof(struct aesd_dev));

    aesd_circular_buffer_init(&aesd_device.buffer);
    mutex_init(&aesd_device.buf_mutex);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    uint8_t index;
    struct aesd_buffer_entry *entry;

    cdev_del(&aesd_device.cdev);

    /* Free every buffptr still held in the circular buffer */
    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.buffer, index) {
        if (entry->buffptr) {
            kfree(entry->buffptr);
            entry->buffptr = NULL;
        }
    }

    /* Free any partial (unterminated) working entry */
    if (aesd_device.working_entry.buffptr) {
        kfree(aesd_device.working_entry.buffptr);
        aesd_device.working_entry.buffptr = NULL;
    }

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
