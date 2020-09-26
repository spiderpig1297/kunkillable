#include "readpid.h"

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>

static int device_open_count = 0;

// file_operation functions for the character device.
static int device_open(struct inode* inode, struct file* file);
static int device_release(struct inode* inode, struct file* file);
static ssize_t device_read(struct file *fs, char *buffer, size_t len, loff_t *offset);
static ssize_t device_write(struct file *fs, const char*buffer, size_t len, loff_t *offset);

LIST_HEAD(kunkillable_procs);

static struct file_operations _file_ops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

inline void make_process_killable(struct task_struct *ts)
{   
    ts->signal->flags &= ~SIGNAL_UNKILLABLE;
}

inline void make_process_unkillable(struct task_struct *ts)
{
    ts->signal->flags |= SIGNAL_UNKILLABLE;
}

int register_readpid_chrdev(const char* device_name)
{
    return register_chrdev(0, device_name, &_file_ops);
}

void unregister_readpid_chrdev(int major_num, const char* device_name) 
{
    unregister_chrdev(major_num, device_name);
}

static ssize_t device_write(struct file *fs, const char *buffer, size_t len, loff_t *offset)
{
    int pid_as_int = 0;
    pid_t pid = 0;
    struct proc_info *new_proc_info;
    struct task_struct *proc_task_struct;

    // convert the given pid to pid_t
    int conversion_result = kstrtoint(buffer, DECIMAL_BASE, &pid_as_int);
    if ((-EINVAL == conversion_result) || (-ERANGE == conversion_result)) {
        printk(KERN_DEBUG "kunkillable: failed to convert buffer to pid_t.\n");
        return -EIO;
    }
    pid = (pid_t)pid_as_int;

    // get the process' task_struct
    proc_task_struct = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (NULL == proc_task_struct) {
        printk(KERN_ERR "kunkillable: failed to find process with pid %d\n", pid);
        return -EIO;
    }

    // make the process unkillable
    make_process_unkillable(proc_task_struct);

    // add the process information to the list for us to be able to restore it when unloaded.
    new_proc_info = (struct proc_info*)kmalloc(sizeof(struct proc_info), GFP_KERNEL);
    if (NULL == pid) {
        printk(KERN_WARNING "kunkillable: failed to add pid %d to the list, process will be forever unkillable.\n", pid);
        return -EIO;
    }

    new_proc_info->pid = pid;
    INIT_LIST_HEAD(&new_proc_info->l_head);
    list_add_tail(&new_proc_info->l_head, &kunkillable_procs);

    printk(KERN_INFO "kunkillable: process %d is now unkillable\n", pid);

    return len;
}

static int device_open(struct inode* inode, struct file* file)
{
    if (device_open_count) {
        return -EBUSY;
    }

    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode* inode, struct file* file)
{
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *fs, char *buffer, size_t len, loff_t *offset)
{ 
    return len;
}