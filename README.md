# __kunkillable__

_kunkillable_ is an LKM (loadable kernel module) that makes userland processes unkillable.

## __TL;DR__

**_NOTE_**: this section is a short version of how the module works. for a full version, see [How It Works](#how-it-works).

_kunkillable_ takes advantage of the signal flags of a given task_struct, and adds the flag _SIGNAL_UNKILLABLE_ hence making it unkillable.

When the module is unloaded, the flag is removed and the process becomes killable again.

## __How It Works__

Before we'll discuss on the MO of the _kunkillable_ module, lets see what happens when we send a user-mode signal to a process:

        -----------------
        | kill -9 25327 |
        -----------------
            |
            |   user runs kill command to send a signal for a process, intiating a 
            |   call to sys_kill() system call
            |
        --------------
        | sys_kill() |
        --------------
            |
            |   system call sys_kill() is called, initiating a sequence of internal functions 
            |   called is initiated, with kill_something_info as the first one
            |
        -------------------------     -------------------     -------------------------
        | kill_something_info() | --> | kill_pid_info() | --> | group_send_sig_info() | 
        -------------------------     -------------------     -------------------------
                                                                    |
                                                                    |
            _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ |
            |
        ----------------------     -----------------      -------------------
        | do_send_sig_info() | --> | send_signal() |  --> | __send_signal() |
        ----------------------     -----------------      -------------------
                                                                    |
            _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ |
            |   
            |   evantually, __send_signal() is called.
            |   __send_signal is the last function in the chain whose responsible for the actual
            |   send of the signal.
            |
        --------------------
        | prepare_signal() |
        --------------------
            |
            |   as part of the signal prepartion, prepare_signal() is called and in turns calls two
            |   interseting function!
            |
        -----------------
        | sig_ignored() |
        -----------------
            |
            |
            |
        ----------------------
        | sig_task_ignored() |
        ----------------------

                sig_task_ignored() is the function that responsible for checking if the process should
                ignore the signal that we want to send, according to its task_struct's flags.

Let's take a look at the function sig_task_ignored():

**_NOTE_**: you can find the rest of the screenshots under [docs/](https://github.com/spiderpig1297/kunkillable/tree/master/docs) directory.

![Alt text](https://github.com/spiderpig1297/kunkillable/blob/master/docs/10_sig_task_ignored.png)

### _SIGNAL_UNKILLABLE_
As we can see in line 85, the kernel reads the task_struct's signal flags to find if _SIGNAL_UNKILLABLE_ is defined. If so - ___true is returned, and the signal is being ignored - hence our process becomes unkillable.

All we need to do - is to find the task_struct of the process we want to turn unkillable, and add _SIGNAL_UNKILLABLE_ flag to it.
