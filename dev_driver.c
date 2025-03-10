#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/input.h>

#define PROC_FILENAME "mouse_brightness"
static struct proc_dir_entry *proc_file;

// Store mouse click state
static char mouse_click_state[10] = "none";

static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(ubuf, count, ppos, mouse_click_state, strlen(mouse_click_state));
}

// Callback when a mouse event happens
static void mouse_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
    if (type == EV_KEY) {  // Key press event
        if (code == BTN_LEFT && value) 
            strcpy(mouse_click_state, "left\n");
        else if (code == BTN_RIGHT && value) 
            strcpy(mouse_click_state, "right\n");
        else if (code == BTN_MIDDLE && value) 
            strcpy(mouse_click_state, "middle\n");
    }
}

// Input device registration
static struct input_handler mouse_handler = {
    .event = mouse_event,
    .name = "mouse_brightness_handler",
};

// Module initialization
static int __init mouse_init(void)
{
    proc_file = proc_create(PROC_FILENAME, 0666, NULL, &(struct proc_ops){ .proc_read = proc_read });
    if (!proc_file) return -ENOMEM;

    printk(KERN_INFO "Mouse driver loaded. Read /proc/mouse_brightness\n");
    return 0;
}

// Cleanup function
static void __exit mouse_exit(void)
{
    remove_proc_entry(PROC_FILENAME, NULL);
    printk(KERN_INFO "Mouse driver unloaded.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Custom");
MODULE_DESCRIPTION("Mouse Driver for Brightness Control");

module_init(mouse_init);
module_exit(mouse_exit);
