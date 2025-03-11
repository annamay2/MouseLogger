#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "mouse_logger_1"
#define BUFFER_SIZE 256

static int major_number;
static struct cdev mouse_cdev;
static struct class *mouse_class;
static struct input_handler mouse_handler;
static struct input_dev *input_device;
static char event_buffer[BUFFER_SIZE];
static int buffer_pos = 0;
static DEFINE_MUTEX(buffer_lock);

// Write event to buffer
static void log_event(const char *event) {
    mutex_lock(&buffer_lock);
    snprintf(event_buffer, BUFFER_SIZE, "%s\n", event);
    buffer_pos = strlen(event_buffer);
    mutex_unlock(&buffer_lock);

    printk(KERN_INFO "Mouse Logger: Logged event - %s\n", event);
}

// Read function for user-space
static ssize_t mouse_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset) {
    if (*offset >= buffer_pos) return 0; // No new data

    mutex_lock(&buffer_lock);
    if (copy_to_user(user_buffer, event_buffer, buffer_pos)) {
        mutex_unlock(&buffer_lock);
        return -EFAULT;
    }
    mutex_unlock(&buffer_lock);

    *offset += buffer_pos;
    return buffer_pos;
}

// Open function (not much needed here)
static int mouse_open(struct inode *inode, struct file *file) {
    return 0;
}

// Release function
static int mouse_release(struct inode *inode, struct file *file) {
    return 0;
}

// File operations struct
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = mouse_read,
    .open = mouse_open,
    .release = mouse_release,
};

// Mouse event callback
static void mouse_event(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
printk(KERN_INFO "Mouse Logger: Received event - type: %u, code: %u, value: %d\n", type, code, value);

if (type == EV_KEY && value) {
        if (code == BTN_LEFT) log_event("Left Click");
        else if (code == BTN_RIGHT) log_event("Right Click");
        else if (code == BTN_MIDDLE) log_event("Middle Click");
    }
}

// Connect function (called when a device is found)
static int mouse_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id) {
    struct input_handle *handle;
    
    // Only attach to mouse devices
    if (!test_bit(EV_KEY, dev->evbit) || !test_bit(BTN_LEFT, dev->keybit)) {
        printk(KERN_INFO "Mouse Logger: Skipping non-mouse device\n");
        return -ENODEV;
    }

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle) return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "mouse_logger_1";

    if (input_register_handle(handle)) {
        kfree(handle);
        return -EINVAL;
    }

    if (input_open_device(handle)) {
        input_unregister_handle(handle);
        kfree(handle);
        return -EINVAL;
    }

    printk(KERN_INFO "Mouse Logger: Connected to device %s\n", dev->name);
    return 0;
}

static void mouse_disconnect(struct input_handle *handle) {
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
    printk(KERN_INFO "Mouse Logger: Device disconnected\n");
}

// Input handler setup
static const struct input_device_id mouse_ids[] = {
    { .driver_info = 1 },  // Matches all devices
    { },
};

MODULE_DEVICE_TABLE(input, mouse_ids);

// Input device registration
static struct input_handler mouse_handler = {
    .event = mouse_event,
    .name = "mouse_logger_1_handler",
};

// Module initialization
static int __init mouse_init(void) {
    dev_t dev;

    // Allocate major number
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) return -1;
    major_number = MAJOR(dev);

    // Register character device
    cdev_init(&mouse_cdev, &fops);
    if (cdev_add(&mouse_cdev, dev, 1) < 0) return -1;

    // Create class and device entry in /dev/
    mouse_class = class_create(DEVICE_NAME);
    device_create(mouse_class, NULL, dev, NULL, DEVICE_NAME);

    printk(KERN_INFO "Mouse Logger Loaded. Use: cat /dev/%s\n", DEVICE_NAME);
    //This line breaks it for some reason
    //input_register_handler(&mouse_handler);
    return 0;
}

// Module cleanup
static void __exit mouse_exit(void) {
    dev_t dev = MKDEV(major_number, 0);

    device_destroy(mouse_class, dev);
    class_destroy(mouse_class);
    cdev_del(&mouse_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Mouse Logger Unloaded.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Custom");
MODULE_DESCRIPTION("Mouse Logger using /dev Interface");

module_init(mouse_init);
module_exit(mouse_exit);
