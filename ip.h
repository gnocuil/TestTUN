#ifndef __IP_H__
#define __IP_H__

#include <net/if.h>

int ip_type(char *ipheader)
{
	return (ipheader[0] >> 4);
}

struct ipv6hdr {
    unsigned long flow;   /* 4 bits version, 8 bits TC, 20 bits flow-ID */
    unsigned short plen;   /* payload length */
    unsigned char  nxt;    /* next header */
    unsigned char  hlim;   /* hop limit */
    struct in6_addr src;      /* source address */
    struct in6_addr dst;      /* destination address */
};

#endif
