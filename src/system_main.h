#ifndef SYSTEM_MAIN_H
#define SYSTEM_MAIN_H

#include <linux/netfilter_ipv4.h>

#define SYSTEM_NAME         "system_watchdog"
#define SYSTEM_VERSION      "0.4.0"
#define SYSTEM_AUTHOR       "Roman E. Chechnev"
#define SYSTEM_DESCRIPTION  "System watchdog"
#define SYSTEM_LICENSE      "GPL"

#define UDP_DST_PORT    911
#define UDP_MAGIC       0x3931310a // 911\n

#define MGMT_IF_ICMP_TIMEOUT_SEC    0
#define MGMT_IF_DOWN_TIMEOUT_SEC    60
#define MGMT_IF_IDLE_TIMEOUT_SEC    300

typedef int             int32_t;
typedef unsigned int    uint32_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;

typedef struct system_watchdog_t system_watchdog_t;
typedef enum   system_state_t    system_state_t;

struct system_watchdog_t
{
    struct nf_hook_ops  nfho_ipv4;
    struct nf_hook_ops  nfho_arp;
    struct task_struct  *thread1;
    char                *mgmt_if_name;
    struct net_device   *mgmt_dev;
    uint16_t            mgmt_if_icmp_timeout_sec;
    uint16_t            mgmt_if_icmp_timeout_left_sec;
    uint16_t            mgmt_if_down_timeout_sec;
    uint16_t            mgmt_if_down_timeout_left_sec;
    uint16_t            mgmt_if_idle_timeout_sec;
    uint16_t            mgmt_if_idle_timeout_left_sec;
    uint32_t            mgmt_if_last_packets_count;
    struct kobject      *root_kobject;
};

#endif

