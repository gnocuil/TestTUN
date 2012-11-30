#ifndef __NETWORK_H__
#define __NETWORK_H__

int set_mtu(char *interface_name, unsigned mtu);//ifconfig interface_name mtu xxx

int interface_up(char *interface_name);//ifconfig interface_name up

int set_ipaddr(char *interface_name, char *ip);

int route_add(char * interface_name);//ip route del default; ip route add default dev interface_name

#endif
