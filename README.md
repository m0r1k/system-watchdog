# system\_watchdog

Linux kernel module for watchdog and reboot test labs

## Install

```
sudo apt install ./system-watchdog_0.4.0+.deb
```

## Setup

please edit /etc/modprobe.d/system\_watchdog.conf, setup 'mgmt\_if\_name' param into the managment interface name (interface for monitoring), example:

```
options system_watchdog mgmt_if_name=enp4s0
```

you can change the other params too, example:

```
options system_watchdog mgmt_if_name=enp4s0 mgmt_if_icmp_timeout_sec=60 mgmt_if_down_timeout_sec=60 mgmt_if_idle_timeout_sec=300
```

please check that module exist in /etc/modules-load.d/system\_watchdog.conf for autoload after reboot

```
cat /etc/modules-load.d/system_watchdog.conf
system_watchdog

```

load the module by hands

```
sudo modprobe system_watchdog
```

## Usage:

### Emergency UDP reboot

send an UDP packet to the 911 port with payload 911 to the managment interface and machine will be rebooted

```
echo "911" | nc -u 192.168.1.1 911

```

you can use sysfs to change params on fly:

### Interface ICMP protection

if not 0 and managment interface will not receive ICMP packets, after this time the machine will be rebooted

```
echo 60 > /sys/kernel/system_watchdog/mgmt_if_icmp_timeout_sec

```

### Interface idle protection

if not 0 and managment interface will not receive IP and ARP packets, after this time the machine will be rebooted

```
echo 300 > /sys/kernel/system_watchdog/mgmt_if_idle_timeout_sec

```

### Interface down protection

if not 0 and managment interface will have not 'UP' state, after this time the machine will be rebooted

```
echo 60 > /sys/kernel/system_watchdog/mgmt_if_down_timeout_sec

```

## Notes

after installation the sources available in the /usr/src/system-watchdog/src/

use:

```
sudo make -C /usr/src/system-watchdog/src/ build   ## to build the kernel module
sudo make -C /usr/src/system-watchdog/src/ install ## to install the kernel module
sudo make -C /usr/src/system-watchdog/src/ prep    ## to install the kernel headers
make deb REPONAME=bullseye                         ## to build the deb package
```

```
sudo dmesg ## to show current state

[ 5375.924148] system_watchdog (0.1) attempt to load, mgmt_if_name: 'enp4s0', mgmt_if_icmp_timeout_sec: 60, mgmt_if_down_timeout_sec: 60, mgmt_if_idle_timeout_sec: 300
[ 5375.925627] system_watchdog (0.1) kernel module loaded

[root@DEV2 /home/morik]# echo "60" >  /sys/kernel/system_watchdog/mgmt_if_icmp_timeout_sec
[  172.537702] managment interface 'enp4s0' icmp missing, reboot after: 59 sec(s)
[  237.514531] managment interface 'enp4s0' icmp missing, reboot after: 58 sec(s)
[  238.514070] managment interface 'enp4s0' icmp missing, reboot after: 57 sec(s)
[  239.513612] managment interface 'enp4s0' icmp missing, reboot after: 56 sec(s)
[  240.513096] managment interface 'enp4s0' icmp missing, reboot after: 55 sec(s)
[root@DEV2 /home/morik]# echo "0" >  /sys/kernel/system_watchdog/mgmt_if_icmp_timeout_sec

```

## Thanks

