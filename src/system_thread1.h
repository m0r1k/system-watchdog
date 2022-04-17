#ifndef SYSTEM_THREAD1_H
#define SYSTEM_THREAD1_H

#include "system_main.h"

int32_t thread1_tick_1s(
    system_watchdog_t   *a_engine
);

int32_t thread1_fn(
    void            *a_priv
);

void cleanup_thread1(
    system_watchdog_t   *a_engine
);

int32_t init_thread1(
    system_watchdog_t   *a_engine
);

#endif

