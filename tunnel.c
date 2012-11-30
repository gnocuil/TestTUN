#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <errno.h>
#include <net/route.h>

#include <tunnel.h>

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

/**
 *  route add 0.0.0.0/0 dev interface_name
 */
int route_add(char * interface_name)
{
    int skfd;
    struct rtentry rt;

    struct sockaddr_in dst;
    struct sockaddr_in gw;
    struct sockaddr_in genmask;

    memset(&rt, 0, sizeof(rt));

    genmask.sin_addr.s_addr = inet_addr("255.0.0.0");

    bzero(&dst,sizeof(struct sockaddr_in));
    dst.sin_family = PF_INET;
    dst.sin_addr.s_addr = inet_addr("0.0.0.0");

    rt.rt_metric = 0;
    rt.rt_dst = *(struct sockaddr*) &dst;
    rt.rt_genmask = *(struct sockaddr*) &genmask;

    rt.rt_dev = interface_name;
    rt.rt_flags = RTF_UP | RTF_HOST ;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(ioctl(skfd, SIOCADDRT, &rt) < 0) 
    {
        printf("Error route add :%m\n", errno);
        return -1;
    }
}
int main(int argc, char *argv[])
{
#ifdef __linux
	puts("Running in linux!");
#elif __APPLE__
	puts("Running in mac os!");
#else
	puts("unknown OS!");
	return 1;
#endif
    int tun, ret;
    char tun_name[IFNAMSIZ];
    unsigned char buf[4096];
    unsigned char ip[4];

    tun_name[0] = '\0';
    tun = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun < 0) 
    {
        return 1;
    }
    printf("TUN name is %s\n", tun_name);

    //激活虚拟网卡增加到虚拟网卡的路由
    interface_up(tun_name);
    //set_ipaddr(tun_name, "10.0.0.1");
    route_add(tun_name);

    while (1) {//printf("loop!\n");

        ret = read(tun, buf, sizeof(buf));
        
        printf("read %d bytes\n", ret);
        int i;
        for(i=0;i<ret;i++)
        {
          printf("%02x ",buf[i]);
        }
        printf("\n");
        
        if (ret < 0)
            break;
#define ETH_LEN 0
        memcpy(ip, &buf[ETH_LEN + 12], 4);
        memcpy(&buf[ETH_LEN + 12], &buf[ETH_LEN + 16], 4);
        memcpy(&buf[ETH_LEN + 16], ip, 4);
        buf[ETH_LEN + 20] = 0;
        *((unsigned short*)&buf[ETH_LEN + 22]) += 8;
        
        ret = write(tun, buf, ret);
        //printf("write %d bytes\n", ret);
    }

    return 0;
}
