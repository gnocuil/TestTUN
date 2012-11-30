#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <sys/types.h>
#include <errno.h>
#include <net/route.h>

#ifdef __linux
#include <linux/if_tun.h>
#elif __APPLE__
//TODO : mac os
#else
//TODO : unknown os
#endif

#include <tunnel.h>

//ifconfig interface_name mtu xxx
static int set_mtu(char *interface_name, unsigned mtu)
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
        printf("Error up %s :%m\n",interface_name, errno);
        return -1;
    }

    return 0;
}

//ifconfig interface_name up
static int interface_up(char *interface_name) 
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
/*
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
*/
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

//ip route add default dev interface_name
static int route_add(char * interface_name)
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


int main(int argc, char *argv[])
{
    char tun_name[IFNAMSIZ] = "lw4over6";
    unsigned char buf[4096];
    unsigned char ip[4];

    int tun = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun < 0) 
    {
        return 1;
    }
    printf("TUN name is %s\n", tun_name);

    set_mtu(tun_name, 50000);//set mtu to 50000
    interface_up(tun_name);//interface up
    //set_ipaddr(tun_name, "10.0.0.1");
    route_add(tun_name);

    int len;
    while (1) {//printf("loop!\n");

        len = read(tun, buf, sizeof(buf));
        
        printf("read %d bytes\n", len);
        int i;
        for(i=0;i<len;i++)
        {
          printf("%02x ",buf[i]);
        }
        printf("\n");
        
        if (len < 0)
            break;
#define ETH_LEN 0
        memcpy(ip, &buf[ETH_LEN + 12], 4);
        memcpy(&buf[ETH_LEN + 12], &buf[ETH_LEN + 16], 4);
        memcpy(&buf[ETH_LEN + 16], ip, 4);
        buf[ETH_LEN + 20] = 0;
        *((unsigned short*)&buf[ETH_LEN + 22]) += 8;
        
        len = write(tun, buf, len);
        //printf("write %d bytes\n", ret);
    }

    return 0;
}
