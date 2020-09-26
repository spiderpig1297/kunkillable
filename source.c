#include "readpid.h"

#include <linux/types.h>
#include <linux/pid.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spiderpig");
MODULE_VERSION("1.0.0");

static const char* readpid_chrdev_name = "kprochide_readpid";
static int readpid_chrdev_major_num;

static int __init mod_init(void)
{
    printk(KERN_INFO "unkillable: LKM loaded\n");

    readpid_chrdev_major_num = register_readpid_chrdev(readpid_chrdev_name);
    if (readpid_chrdev_major_num < 0) {
        printk(KERN_ERR "kunkillable: failed to register char device (%d)\n", readpid_chrdev_major_num);
        return -EBUSY;
    }

    return 0;
}

static void __exit mod_exit(void)
{
    // unregister character device
    unregister_readpid_chrdev(readpid_chrdev_major_num, readpid_chrdev_name);
    printk(KERN_INFO "kunkillable: unregistered char device %d\n", readpid_chrdev_major_num);

    struct list_head *pos = NULL;
    struct list_head *tmp = NULL;
    struct proc_info *pid = NULL;
    struct task_struct *proc_task_struct = NULL;

    list_for_each_safe(pos, tmp, &kunkillable_procs) {
        pid = list_entry(pos, struct proc_info, l_head);
        if (NULL == pid) {
            continue;
        }

        // get the process' task_struct
        proc_task_struct = pid_task(find_vpid(pid->pid), PIDTYPE_PID);
        if (NULL == proc_task_struct) {
            printk(KERN_ERR "kunkillable: failed to get task_struct of pid %d\n", pid->pid);
            goto remove_item;
        }

        // make the process killable again
        make_process_killable(proc_task_struct);

        printk(KERN_INFO "kunkillable: process %d is now killable again\n", pid->pid);

remove_item:
        list_del(pos);
        kfree(pid);
    }

    printk(KERN_INFO "kunkillable: LKM unloaded succsefully\n");
}

module_init(mod_init);
module_exit(mod_exit);