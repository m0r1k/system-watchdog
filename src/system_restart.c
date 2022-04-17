#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/watchdog.h>
#include <linux/reboot.h>

#include "system_restart.h"

void do_emergency_restart(
    system_watchdog_t *a_engine)
{
    printk(KERN_EMERG "REBOOT by '%s'\n", SYSTEM_NAME);
    emergency_restart();
}

