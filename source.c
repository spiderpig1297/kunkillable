#include <linux/pid.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spiderpig");
MODULE_VERSION("1.0.0");

static const int PID = 1727;

static int __init mod_init(void)
{
    printk(KERN_INFO "unkillable: LKM loaded\n");

    struct task_struct* proc = pid_task(find_vpid(PID), PIDTYPE_PID);
    if (NULL == proc) {
        printk(KERN_ERR "kunkillable: failed to find process with pid %d\n", PID);
        return -EIO;
    }

    printk(KERN_INFO "unkillable: parent PID is %d\n", proc->parent->pid);

    printk(KERN_INFO "unkillable: signal flags are %d\n", proc->signal->flags);
    proc->signal->flags |= SIGNAL_UNKILLABLE;
    printk(KERN_INFO "unkillable: signal flags are %d\n", proc->signal->flags);

    return 0;
}

static void __exit mod_exit(void)
{
    printk(KERN_INFO "unkillable: LKM unloaded succsefully\n");
}

module_init(mod_init);
module_exit(mod_exit);