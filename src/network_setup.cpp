#include "container.h"

void Container::addattr_l(nlmsghdr *n, int max_length, __u16 type, const void *data, __u16 data_length) {
    __u16 attr_length = RTA_LENGTH(data_length);

    __u32 new_length = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(attr_length);
    if (new_length > max_length) {
        std::cerr << "Cannot add attribute. Size " << new_length << " exceeded maxlen " << max_length << ".\n";
        exit(1);
    }
    rtattr *rta;
    rta = NLMSG_TAIL(n);
    rta->rta_type = type;
    rta->rta_len = attr_length;
    if (data_length)
        memcpy(RTA_DATA(rta), data, data_length);

    n->nlmsg_len = new_length;
}

rtattr * Container::add_attr_nest(nlmsghdr *n, int max_length, __u16 type) {
    rtattr *nest = NLMSG_TAIL(n);

    addattr_l(n, max_length, type, NULL, 0);
    return nest;
}

void Container::add_attr_nest_end(nlmsghdr *n, rtattr *nest) {
    nest->rta_len = (char *)(NLMSG_TAIL(n)) - (char *)(nest);
}

ssize_t Container::read_response(int sock_fd, msghdr *msg, char **response) {
    iovec *iov = msg->msg_iov;
    iov->iov_base = *response;
    iov->iov_len = MAX_PAYLOAD;

    ssize_t resp_len = recvmsg(sock_fd, msg, 0);

    if (resp_len == 0) {
        std::cerr << "EOF on netlink." << '\n';
        exit(1);
    }

    if (resp_len < 0) {
        std::cerr << "Netlink receive error." << strerror(errno) << '\n';
        exit(1);
    }

    return resp_len;
}

void Container::check_response(int sock_fd) {
    iovec iov;
    msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    char *resp = (char *) malloc(MAX_PAYLOAD);

    ssize_t resp_len = read_response(sock_fd, &msg, &resp);

    nlmsghdr *hdr = (nlmsghdr *) resp;
    int nlmsglen = hdr->nlmsg_len;
    int datalen = nlmsglen - sizeof(*hdr);

    if (datalen < 0 || nlmsglen > resp_len) {
        if (msg.msg_flags & MSG_TRUNC) {
            std::cerr << "Received truncated message." << '\n';
            exit(1);
        }
        std::cerr << "malformed message: nlmsg_len = " << nlmsglen << '\n';
        exit(1);
    }

    if (hdr->nlmsg_type == NLMSG_ERROR) {
        nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(hdr);

        if (datalen < sizeof(struct nlmsgerr)) {
            std::cerr << "ERROR truncated!" << '\n';
        }

        if(err->error) {
            errno = -err->error;
            std::cerr << "RTNETLINK: " << strerror(errno) << '\n';
            exit(1);
        }
    }

    free(resp);
}

int Container::create_socket(int domain, int type, int protocol) {
    int loc_sock_fd = socket(domain, type, protocol);
    if (loc_sock_fd < 0) {
        std::cerr << "Failed to create a socket for isolated network namespace." << '\n';
        exit(1);
    }
    return loc_sock_fd;
}

void Container::send_nlmsg(int sock_fd, struct nlmsghdr *n)
{
    struct iovec iov = {
            .iov_base = n,
            .iov_len = n->nlmsg_len
    };

    msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;


    n->nlmsg_seq++;

    ssize_t status = sendmsg(sock_fd, &msg, 0);
    if (status < 0) {
        std::cerr << "Cannot talk to rtnetlink: " << strerror(errno) << '\n';
        exit(1);
    }
    check_response(sock_fd);
}

int Container::get_netns_fd(int pid) {
    char path[256];
    sprintf(path, "/proc/%d/ns/net", pid);

    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        std::cerr << "Cannot read netns file: " << path << ": "<< strerror(errno) << '\n';
        exit(1);
    }
    return fd;
}

void Container::if_up(char *ifname, char *ip, char *netmask) {
    int sock_fd = create_socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifreq));
    strncpy(ifr.ifr_name, ifname, strlen(ifname));

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = 0;

    char *p = (char *) &saddr;

    saddr.sin_addr.s_addr = inet_addr(ip);
    memcpy(((char *)&(ifr.ifr_addr)), p, sizeof(sockaddr));
    if (ioctl(sock_fd, SIOCSIFADDR, &ifr)) {
        std::cerr << "Cannot set IP address: " << ifname << ", " << ip << ": " << strerror(errno) << '\n';
        exit(1);
    }


    saddr.sin_addr.s_addr = inet_addr(netmask);
    memcpy(((char *)&(ifr.ifr_addr)), p, sizeof(sockaddr));
    if (ioctl(sock_fd, SIOCSIFNETMASK, &ifr)) {
        std::cerr << "Cannot set netmask for address " << ifname << ", " << netmask << ": " << strerror(errno) << '\n';
        exit(1);
    }

    ifr.ifr_flags |= IFF_UP | IFF_BROADCAST |
                     IFF_RUNNING | IFF_MULTICAST;

    if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr)) {
        std::cerr << "Cannot set flags for address " << ifname << ", " << ip << ": " << strerror(errno) << '\n';
        exit(1);
    }

    close(sock_fd);
}

void Container::create_veth(int sock_fd, char *ifname, char *peername) {
    __u16 flags =
            NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
    nl_req req;

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg)),
    req.n.nlmsg_flags = flags;
    req.n.nlmsg_type = RTM_NEWLINK;
    req.i.ifi_family = PF_NETLINK;

    nlmsghdr *n = &req.n;
    int maxlen = sizeof(req);

    addattr_l(n, maxlen, IFLA_IFNAME, ifname, strlen(ifname) + 1);

    rtattr *linfo =
            add_attr_nest(n, maxlen, IFLA_LINKINFO);
    addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "veth", 5);

    rtattr *linfodata =
            add_attr_nest(n, maxlen, IFLA_INFO_DATA);

    rtattr *peerinfo =
            add_attr_nest(n, maxlen, VETH_INFO_PEER);
    n->nlmsg_len += sizeof(ifinfomsg);
    addattr_l(n, maxlen, IFLA_IFNAME, peername, strlen(peername) + 1);
    add_attr_nest_end(n, peerinfo);

    add_attr_nest_end(n, linfodata);
    add_attr_nest_end(n, linfo);

    send_nlmsg(sock_fd, n);
}

void Container::move_if_to_pid_netns(int sock_fd, char *ifname, int netns) {
    std::cout << "Creating VETH device..." << '\n';
    nl_req req;
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_type = RTM_NEWLINK;
    req.i.ifi_family = PF_NETLINK;


    addattr_l(&req.n, sizeof(req), IFLA_NET_NS_FD, &netns, 4);
    addattr_l(&req.n, sizeof(req), IFLA_IFNAME,
              ifname, strlen(ifname) + 1);
    send_nlmsg(sock_fd, &req.n);
}

void Container::setup_netns(int child_pid) {
    char *veth = strdup("veth0"); //strdup(containerID.c_str());
    char *vpeer = strdup("veth1");
    char *veth_addr = strdup("10.1.1.1");
    char *vpeer_addr = strdup("10.1.1.2");
    char* netmask = strdup("255.255.255.0");


    int sock_fd = create_socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);

    create_veth(sock_fd, veth, vpeer);


    if_up(veth, veth_addr, netmask);


    int mynetns = get_netns_fd(getpid());
    int child_netns = get_netns_fd(child_pid);

    move_if_to_pid_netns(sock_fd, vpeer, child_netns);

    if (setns(child_netns, CLONE_NEWNET)) {
        std::cerr << "Cannot setns for child at pid " << child_pid << ": " << strerror(errno) << '\n';
        exit(1);
    }

    if_up(vpeer, vpeer_addr, netmask);

    if (setns(mynetns, CLONE_NEWNET)) {
        std::cerr << "Cannot restore previous netns: " << strerror(errno) << '\n';
        exit(1);
    }

    free(veth);
    free(vpeer);
    free(veth_addr);
    free(vpeer_addr);
    free(netmask);
    close(sock_fd);
}

