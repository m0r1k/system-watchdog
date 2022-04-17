#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/netdevice.h>

#include "system_restart.h"
#include "system_thread1.h"

int32_t check_netdev_state_1s(
    system_watchdog_t   *a_engine)
{
    int32_t             res, err = 0;
    struct net_device   *dev;

    if (!strlen(a_engine->mgmt_if_name)){
        // mgmt name is not defined
        goto out;
    }

    // reset mgmt_dev (for update below)
    a_engine->mgmt_dev = NULL;

    read_lock(&dev_base_lock);

    dev = first_net_device(&init_net);
    while (dev) {
        // printk(KERN_INFO "found [%s]\n", dev->name);
        if (strcmp(dev->name, a_engine->mgmt_if_name)){
            // not mgmt interface
            dev = next_net_device(dev);
            continue;
        }

        // update mgmt dev
        a_engine->mgmt_dev = dev;

        // check state
        res = netif_running(dev);
        if (res){
            // all ok, reset timer
            a_engine->mgmt_if_down_timeout_left_sec
                = a_engine->mgmt_if_down_timeout_sec;
        }

        // done
        break;
    }

    read_unlock(&dev_base_lock);

out:
    return err;
}

int32_t thread1_tick_1s(
    system_watchdog_t   *a_engine)
{
    int32_t delta, res, err = 0;

    // printk(KERN_INFO "thread1 tick\n");

    res = check_netdev_state_1s(a_engine);
    if (res){
        err = res;
    }

    // check icmp
    do {
        if (!a_engine->mgmt_dev){
            // mgmt not found
            break;
        }
        if (!a_engine->mgmt_if_icmp_timeout_sec){
            // disabled
            a_engine->mgmt_if_icmp_timeout_left_sec
                = a_engine->mgmt_if_icmp_timeout_sec;
            break;
        }
        delta = a_engine->mgmt_if_icmp_timeout_sec
            - a_engine->mgmt_if_icmp_timeout_left_sec;
        if (10 < delta){
            printk(KERN_WARNING "managment interface '%s' icmp missing,"
                " reboot after: %u sec(s)"
                "\n",
                a_engine->mgmt_dev->name,
                a_engine->mgmt_if_icmp_timeout_left_sec
            );
        }
        if (a_engine->mgmt_if_icmp_timeout_left_sec){
            // still have a time
            a_engine->mgmt_if_icmp_timeout_left_sec--;
        } else {
            do_emergency_restart(a_engine);
        }
    } while (0);

    // check if down
    do {
        if (!a_engine->mgmt_dev){
            // mgmt not found
            break;
        }
        if (!a_engine->mgmt_if_down_timeout_sec){
            // disabled
            a_engine->mgmt_if_down_timeout_left_sec
                = a_engine->mgmt_if_down_timeout_sec;
            break;
        }
        delta = a_engine->mgmt_if_down_timeout_sec
            - a_engine->mgmt_if_down_timeout_left_sec;
        if (10 < delta){
            printk(KERN_WARNING "managment interface '%s' is down,"
                " reboot after: %u sec(s)"
                "\n",
                a_engine->mgmt_dev->name,
                a_engine->mgmt_if_down_timeout_left_sec
            );
        }
        if (a_engine->mgmt_if_down_timeout_left_sec){
            // still have a time
            a_engine->mgmt_if_down_timeout_left_sec--;
        } else {
            do_emergency_restart(a_engine);
        }
    } while (0);

    // check if idle
    do {
        if (!a_engine->mgmt_dev){
            // mgmt not found
            break;
        }
        if (!a_engine->mgmt_if_idle_timeout_sec){
            // disabled
            a_engine->mgmt_if_last_packets_count = 0;
            a_engine->mgmt_if_idle_timeout_left_sec
                = a_engine->mgmt_if_idle_timeout_sec;
            break;
        }
        if (a_engine->mgmt_if_last_packets_count){
            // active
            a_engine->mgmt_if_last_packets_count = 0;
            a_engine->mgmt_if_idle_timeout_left_sec
                = a_engine->mgmt_if_idle_timeout_sec;
            break;
        }
        delta = a_engine->mgmt_if_idle_timeout_sec
            - a_engine->mgmt_if_idle_timeout_left_sec;
        if (10 < delta){
            printk(KERN_WARNING "managment interface '%s' is idle,"
                " reboot after: %u sec(s)"
                "\n",
                a_engine->mgmt_dev->name,
                a_engine->mgmt_if_idle_timeout_left_sec
            );
        }
        if (a_engine->mgmt_if_idle_timeout_left_sec){
            // still have a time
            a_engine->mgmt_if_idle_timeout_left_sec--;
            a_engine->mgmt_if_last_packets_count = 0;
        } else {
            do_emergency_restart(a_engine);
        }
    } while (0);

    return err;
}

int32_t thread1_fn(
    void *a_priv)
{
    system_watchdog_t *engine = (system_watchdog_t *)a_priv;

    int32_t res, exit_code = 0;

    while (!kthread_should_stop()){
        res = thread1_tick_1s(engine);
        if (res){
            exit_code = res;
            printk(KERN_INFO "thread1_tick_1s() failed with code: %d\n",
                res
            );
            break;
        }
        msleep(1e6);
    }

    return exit_code;
}

void cleanup_thread1(
    system_watchdog_t *a_engine)
{
    int res;

    if (a_engine->thread1){
        res = kthread_stop(a_engine->thread1);
        printk(KERN_INFO "thread1 stopped, exit code: %d\n",
            res
        );
        a_engine->thread1 = NULL;
    }
}

int32_t init_thread1(
    system_watchdog_t *a_engine)
{
    int32_t err = -1;

    a_engine->thread1 = kthread_create(
        thread1_fn,
        a_engine,
        SYSTEM_NAME
    );
    if (!a_engine->thread1){
        printk(KERN_ERR "cannot create thread1\n");
        goto fail;
    }

    wake_up_process(a_engine->thread1);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

