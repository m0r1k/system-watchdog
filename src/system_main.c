#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/moduleparam.h>

#include "system_main.h"

MODULE_LICENSE(SYSTEM_LICENSE);
MODULE_AUTHOR(SYSTEM_AUTHOR);
MODULE_DESCRIPTION(SYSTEM_DESCRIPTION);

#include "system_netfilter.h"
#include "system_thread1.h"
#include "system_sysfs.h"

// global is bad!
static system_watchdog_t g_engine;

// module params
static char    *mgmt_if_name            = "";
static int32_t mgmt_if_icmp_timeout_sec = MGMT_IF_ICMP_TIMEOUT_SEC;
static int32_t mgmt_if_down_timeout_sec = MGMT_IF_DOWN_TIMEOUT_SEC;
static int32_t mgmt_if_idle_timeout_sec = MGMT_IF_IDLE_TIMEOUT_SEC;

module_param(mgmt_if_name,              charp, S_IRUGO);
module_param(mgmt_if_icmp_timeout_sec,  int,   S_IRUGO);
module_param(mgmt_if_down_timeout_sec,  int,   S_IRUGO);
module_param(mgmt_if_idle_timeout_sec,  int,   S_IRUGO);

MODULE_PARM_DESC(mgmt_if_name,             "interface name");
MODULE_PARM_DESC(mgmt_if_icmp_timeout_sec, "time");
MODULE_PARM_DESC(mgmt_if_down_timeout_sec, "time");
MODULE_PARM_DESC(mgmt_if_idle_timeout_sec, "time");

static int32_t __init system_init(void)
{
    int32_t         res, err = -1;
    system_watchdog_t   *engine  = &g_engine;

    printk(KERN_INFO "%s (%s) attempt to load,"
        " mgmt_if_name: '%s',"
        " mgmt_if_icmp_timeout_sec: %u,"
        " mgmt_if_down_timeout_sec: %u,"
        " mgmt_if_idle_timeout_sec: %u"
        "\n",
        SYSTEM_NAME,
        SYSTEM_VERSION,
        mgmt_if_name,
        mgmt_if_icmp_timeout_sec,
        mgmt_if_down_timeout_sec,
        mgmt_if_idle_timeout_sec
    );

    // copy module params
    engine->mgmt_if_name                  = mgmt_if_name;
    engine->mgmt_if_icmp_timeout_sec      = mgmt_if_icmp_timeout_sec;
    engine->mgmt_if_icmp_timeout_left_sec = mgmt_if_icmp_timeout_sec;
    engine->mgmt_if_down_timeout_sec      = mgmt_if_down_timeout_sec;
    engine->mgmt_if_down_timeout_left_sec = mgmt_if_down_timeout_sec;
    engine->mgmt_if_idle_timeout_sec      = mgmt_if_idle_timeout_sec;
    engine->mgmt_if_idle_timeout_left_sec = mgmt_if_idle_timeout_sec;

    res = init_sysfs(engine);
    if (res){
        err = res;
        printk(KERN_ERR "init_sysfs() failed\n");
        goto fail;
    }

    res = init_nethooks(engine);
    if (res){
        err = res;
        printk(KERN_ERR "init_nethooks() failed\n");
        goto fail;
    }

    res = init_thread1(engine);
    if (res){
        err = res;
        printk(KERN_ERR "init_thread1() failed\n");
        goto fail;
    }

    printk(KERN_INFO "%s (%s) kernel module loaded\n",
        SYSTEM_NAME,
        SYSTEM_VERSION
    );

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

static void __exit system_exit(void)
{
    system_watchdog_t *engine = &g_engine;

    cleanup_thread1(engine);
    cleanup_nethooks(engine);
    cleanup_sysfs(engine);

    printk(KERN_INFO "%s (%s) kernel module unloaded\n",
        SYSTEM_NAME,
        SYSTEM_VERSION
    );
}

module_init(system_init);
module_exit(system_exit);

