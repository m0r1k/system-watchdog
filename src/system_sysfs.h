#ifndef SYSTEM_SYSFS_H
#define SYSTEM_SYSFS_H

#include "system_main.h"

int32_t init_sysfs(
    system_watchdog_t *a_engine
);

void cleanup_sysfs(
    system_watchdog_t *a_engine
);

#endif

