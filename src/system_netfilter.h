#ifndef SYSTEM_NETFILTER_H
#define SYSTEM_NETFILTER_H

#include "system_main.h"

uint32_t netfilter_hook_ipv4(
    void                        *a_priv,
    struct sk_buff              *a_skb,
    const struct nf_hook_state  *a_state
);

int32_t init_nethooks(
    system_watchdog_t   *a_engine
);

void cleanup_nethooks(
    system_watchdog_t *a_engine
);

#endif

