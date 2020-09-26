# __kunkillable__

_kunkillable_ is an LKM (loadable kernel module) that makes userland processes unkillable.

## __TL;DR__

**_NOTE_**: this section is a shorted version of how the module works. for a full version, see [How It Works]
(#how_it_works).


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

    ![Alt text](../docs/10_sig_task_ignored.png)