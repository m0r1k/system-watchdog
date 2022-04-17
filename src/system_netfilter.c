#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#include "system_restart.h"
#include "system_netfilter.h"

uint32_t netfilter_hook_ipv4(
    void                        *a_priv,
    struct sk_buff              *a_skb,
    const struct nf_hook_state  *a_state)
{
    system_watchdog_t       *engine = (system_watchdog_t *)a_priv;
    int32_t             ret     = NF_ACCEPT;

    struct ethhdr       *mac_header;
    struct iphdr        *ip_header;
    struct udphdr       *udp_header;
    struct icmphdr      *icmp_header;

    uint32_t            udp_magic;
    uint16_t            udp_dst_port;
    uint16_t            udp_len;

    if (!a_skb){
        ret = NF_DROP;
        goto out;
    }

    // reset the timer
    if (    engine->mgmt_dev
        &&  engine->mgmt_dev == a_skb->dev)
    {
        engine->mgmt_if_last_packets_count++;
    }

    ip_header  = (struct iphdr *)skb_network_header(a_skb);
    mac_header = (struct ethhdr *)skb_mac_header(a_skb);

    // check icmp
    do {
        if (IPPROTO_ICMP != ip_header->protocol){
            break;
        }

        icmp_header = (struct icmphdr *)(
            (unsigned char *)ip_header + ip_header->ihl * 4
        );

        if (ICMP_ECHO != icmp_header->type){
            break;
        }

        engine->mgmt_if_icmp_timeout_left_sec
            = engine->mgmt_if_icmp_timeout_sec;
    } while (0);

    // check udp
    do {
        if (IPPROTO_UDP != ip_header->protocol){
            break;
        }

        udp_header = (struct udphdr *)(
            (unsigned char *)ip_header + ip_header->ihl * 4
        );

        udp_dst_port = be16_to_cpu(udp_header->dest);
        udp_len      = be16_to_cpu(udp_header->len);

        if (UDP_DST_PORT != udp_dst_port){
            break;
        }

        printk(KERN_INFO "Got an UDP packet\n");
        printk(KERN_INFO "src_ip:   %pI4\n", &ip_header->saddr);
        printk(KERN_INFO "dst_ip:   %pI4\n", &ip_header->daddr);
        printk(KERN_INFO "dst_port: %u\n",   udp_dst_port);
        printk(KERN_INFO "len:      %u\n",   udp_len);

        if (sizeof(udp_magic) != (udp_len - sizeof(struct udphdr))){
            printk(KERN_ERR "wrong udp len: %u, must be: %lu\n",
                udp_len,
                sizeof(udp_magic)
            );
            break;
        }

        udp_magic = be32_to_cpu(
            *(uint32_t *)((unsigned char *)udp_header
                + sizeof(struct udphdr)
            )
        );

        printk(KERN_INFO "udp_magic 0x%8.8x\n", udp_magic);

        if (UDP_MAGIC == udp_magic){
            do_emergency_restart(engine);
        }
    } while(0);

out:
    return ret;
}

static uint32_t netfilter_hook_arp(
    void                        *a_priv,
    struct sk_buff              *a_skb,
    const struct nf_hook_state  *a_state)
{
    system_watchdog_t *engine = (system_watchdog_t *)a_priv;
    int32_t       ret     = NF_ACCEPT;

    if (!a_skb){
        ret = NF_DROP;
        goto out;
    }

    // reset the timer
    if (    engine->mgmt_dev
        &&  engine->mgmt_dev == a_skb->dev)
    {
        engine->mgmt_if_last_packets_count++;
    }

out:
    return ret;
}

int32_t init_nethooks(
    system_watchdog_t *a_engine)
{
    int32_t res, err = -1;

    // netfilter ipv4 hook
    a_engine->nfho_ipv4.hook    = netfilter_hook_ipv4;
    a_engine->nfho_ipv4.hooknum = NF_INET_PRE_ROUTING;
    a_engine->nfho_ipv4.pf      = NFPROTO_IPV4;
    a_engine->nfho_ipv4.priv    = (void *)a_engine;

    // set to highest priority over all other hook functions
    a_engine->nfho_ipv4.priority = NF_IP_PRI_FIRST;

    res = nf_register_net_hook(
        &init_net,
        &a_engine->nfho_ipv4
    );
    if (res){
        err = res;
        printk(KERN_ERR "nf_register_net_hook() failed for ipv4\n");
        goto fail;
    }

    // netfilter arp hook
    a_engine->nfho_arp.hook    = netfilter_hook_arp;
    a_engine->nfho_arp.hooknum = NF_INET_PRE_ROUTING;
    a_engine->nfho_arp.pf      = NFPROTO_ARP;
    a_engine->nfho_arp.priv    = (void *)a_engine;

    // set to highest priority over all other hook functions
    a_engine->nfho_arp.priority = NF_IP_PRI_FIRST;

    res = nf_register_net_hook(
        &init_net,
        &a_engine->nfho_arp
    );
    if (res){
        err = res;
        printk(KERN_ERR "nf_register_net_hook() failed for arp\n");
        goto fail;
    }

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

void cleanup_nethooks(
    system_watchdog_t *a_engine)
{
    nf_unregister_net_hook(&init_net, &a_engine->nfho_ipv4);
    nf_unregister_net_hook(&init_net, &a_engine->nfho_arp);
}

