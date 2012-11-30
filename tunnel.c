#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#ifdef __linux
#include <linux/if_tun.h>
#elif __APPLE__
//TODO : mac os
#else
//TODO : unknown os
#endif

#include "tunnel.h"
#include "network.h"

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
    //set_ipaddr(tun_name, "58.205.200.1");
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
