#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <net/route.h>
#include <net/if_arp.h>

#include "network.h"

int set_mtu(char *interface_name, unsigned mtu)
{
    int fd;
    if ((fd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        printf("Error create socket :%m\n", errno);
        return -1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name,interface_name);
    ifr.ifr_mtu = mtu;

    if(ioctl(fd, SIOCSIFMTU, &ifr) < 0)
    {
        printf("Error set %s mtu :%m\n",interface_name, errno);
        return -1;
    }

    return 0;
}

int set_random_mac(char *interface_name)
{
    int fd;
    if ((fd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        printf("Error create socket :%m\n", errno);
        return -1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name,interface_name);

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    unsigned char addr[6] = {35};
    int i;
    for (i = 0; i < 6; ++i)
        addr[i] = rand() % 256;
    memcpy(&ifr.ifr_hwaddr.sa_data, addr, 6);

    if(ioctl(fd, SIOCSIFHWADDR, &ifr) < 0)
    {
        printf("Error set %s mac address :%m\n",interface_name, errno);
        return -1;
    }

    return 0;
}


int interface_up(char *interface_name) 
{
    int s;

    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        printf("Error create socket :%m\n", errno);
        return -1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name,interface_name);

    short flag;
    flag = IFF_UP;
    if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
    {
        printf("Error up %s :%m\n",interface_name, errno);
        return -1;
    }

    ifr.ifr_ifru.ifru_flags |= flag;

    if(ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
    {
        printf("Error up %s :%m\n",interface_name, errno);
        return -1;
    }

    return 0;
}

int set_ipaddr(char *interface_name, char *ip)
{
    int s;

    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error up %s :%m\n",interface_name, errno);
        return -1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface_name);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = PF_INET;
    inet_aton(ip, &addr.sin_addr);

    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));

    if(ioctl(s, SIOCSIFADDR, &ifr) < 0)
    {
        printf("Error set %s ip :%m\n",interface_name, errno);
        return -1;
    }

    return 0;
}

int route_add(char * interface_name)
{
    int skfd;
    struct rtentry rt;

    struct sockaddr_in dst;
    //struct sockaddr_in gw;
    struct sockaddr_in genmask;

    bzero(&genmask,sizeof(struct sockaddr_in));
    genmask.sin_family = AF_INET;
    genmask.sin_addr.s_addr = inet_addr("0.0.0.0");

    bzero(&dst,sizeof(struct sockaddr_in));
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr("0.0.0.0");

    memset(&rt, 0, sizeof(rt));

    rt.rt_dst = *(struct sockaddr*) &dst;
    rt.rt_genmask = *(struct sockaddr*) &genmask;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(ioctl(skfd, SIOCDELRT, &rt) < 0) 
    {
        //printf("Error route del :%m\n", errno);
        //return -1;
    }

    memset(&rt, 0, sizeof(rt));

    rt.rt_metric = 0;
  
    rt.rt_dst = *(struct sockaddr*) &dst;
    rt.rt_genmask = *(struct sockaddr*) &genmask;

    rt.rt_dev = interface_name;
    rt.rt_flags = RTF_UP;

    //skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(ioctl(skfd, SIOCADDRT, &rt) < 0) 
    {
        printf("Error route add :%m\n", errno);
        return -1;
    }
}
