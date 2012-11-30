#ifndef __TUNNEL_H__
#define __TUNNEL_H__

unsigned char local6addr[128], remote6addr[128];
struct in6_addr local, remote;

char tun_name[IFNAMSIZ];

#define MTU 50000

#define BUF_LEN (MTU + 0)
#define IP6HDR_LEN 40

#endif
