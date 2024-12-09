#ifndef MYDOCKER2_NETWORK_NS_H
#define MYDOCKER2_NETWORK_NS_H

#include <stdio.h>
#include <string.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>
#include <net/if.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#define MAX_PAYLOAD 1024

struct nl_req {
    struct nlmsghdr n;
    struct ifinfomsg i;
    char buf[MAX_PAYLOAD];
};

#define NLMSG_TAIL(nmsg) ((struct rtattr *) (((char *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))





#endif
