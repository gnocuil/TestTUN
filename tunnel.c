#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <netinet/in.h>
#ifdef __linux
#include <linux/if_tun.h>
#elif __APPLE__
//TODO : mac os
#else
//TODO : unknown os
#endif

#include "tunnel.h"
#include "network.h"
#include "ip.h"

static int tun_create(char *dev, int flags)
{
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        printf("Error :%m\n", errno);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags |= flags;

    if (*dev != '\0')
    {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) 
    {
        printf("Error :%m\n", errno);
        close(fd);
        return -1;
    }

    strcpy(dev, ifr.ifr_name);

    return fd;
}

static unsigned char buf0[BUF_LEN + IP6HDR_LEN];

static unsigned char* add_ip6header(char *buf, int *len)
{
    buf -= IP6HDR_LEN;
    memset(buf, 0, IP6HDR_LEN);
    
    struct ipv6hdr *ip6h = (struct ipv6hdr*)buf;
    buf[0] = 0x60;
    ip6h->plen = htons(*len);
    ip6h->nxt = IPPROTO_IPIP;//IPv4 over IPv6 protocol
    ip6h->hlim = 64;     
    ip6h->src = local;
    ip6h->dst = remote;
    
    *len += IP6HDR_LEN;
    return buf;
}

void send6(char *buf, int len)
{
    int sock = socket(PF_INET6, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        printf("Error send6 :%m\n", errno);
        return;
    }
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = remote;
    
    if (sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error sendto 6 :%m\n", errno);
    }
    close(sock);
}

int main(int argc, char *argv[])
{
    strcpy(tun_name, "lw4over6");
    strcpy(local6addr, "2001:da8:bf:19::7");
    strcpy(remote6addr, "2001:da8:bf:19::3");

    inet_pton(AF_INET6, local6addr, &local);
    inet_pton(AF_INET6, remote6addr, &remote);

    int tun = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun < 0) 
    {
        return 1;
    }
    printf("TUN name is %s\n", tun_name);

    //set_random_mac(tun_name);
    set_mtu(tun_name, MTU);//set mtu to MTU
    interface_up(tun_name);//interface up
    //set_ipaddr(tun_name, "58.205.200.1");
    route_add(tun_name);

    int len;
    while (1) {
        char *buf = buf0 + IP6HDR_LEN;
        len = read(tun, buf, BUF_LEN);
        printf("read len=%d\n", len);
        if (ip_type(buf) == 4) {
            buf = add_ip6header(buf, &len);
            send6(buf, len);
        }
//        len = write(tun, buf, len);
    }

    return 0;
}
