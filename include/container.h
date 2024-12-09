#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "network_ns.h"
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sched.h>
#include "vector"
#include <fstream>
#include <map>
#include <unordered_map>
#include <sys/mman.h>
#include <cerrno>
#include <utility>
#include <csignal>
#include <boost/algorithm/string.hpp>
#include "config_parser.h"
#include <filesystem>
#include <iomanip>

#define STACK_SIZE (1024 * 1024)
#define CG_PATH "/sys/fs/cgroup"
#define IMAGES {"alp_minifs", "alp_minifs_extend", "py_alp"}
#define MAX_PAYLOAD 1024

class Container {
private:
    int p_fd[2];
    std::string img;
    std::string containerID;
    std::string cgroup_path;
    std::string sources_path;
    std::string num_of_procs;
    std::string max_memory;
    std::vector<std::string> local_folders;
    bool isRunning;
    pid_t cntr_pid;

    // Stack
    static void* create_stack();
    // User Namespace
    static void setup_user_ns(int user_id, pid_t comm_pid);
    // CGroup
    void add_to_cgroup(pid_t comm_pid);
    std::string setup_cgroup();
    void set_max_pid();
    void set_max_memory();
    // Child process
    static int command_process(void* arg);
    static int spawn_process(int (*child_func)(void *), void *arg_for_commproc);
    // Mount namespace
    static void set_permissions(const std::string& path);
    void setup_mount_ns();
    //Network namespace
    static void addattr_l(struct nlmsghdr *n, int max_length, __u16 type, const void *data, __u16 data_length);
    static rtattr *add_attr_nest(struct nlmsghdr *n, int max_length, __u16 type);
    void add_attr_nest_end(struct nlmsghdr *n, struct rtattr *nest);
    ssize_t read_response(int sock_fd, struct msghdr *msg, char **response);
    void check_response(int sock_fd);
    int create_socket(int domain, int type, int protocol);
    void send_nlmsg(int sock_fd, struct nlmsghdr *n);
    int get_netns_fd(int pid);
    void if_up(char *ifname, char *ip, char *netmask);
    void create_veth(int sock_fd, char *ifname, char *peername);
    void move_if_to_pid_netns(int sock_fd, char *ifname, int netns);
    void setup_netns(int child_pid);
    // Other
    std::string create_system(const std::string& source_path);


public:
    Container(std::string id, bool is_new, std::vector<std::string> local_folders = {}, std::string num_of_procs = "max", std::string max_memory = "max", std::string img = "alp_minifs");
    void initialize();
    void run();
    void stop();
    void remove();

    [[nodiscard]] std::string getID() const;
    [[nodiscard]] pid_t get_PID() const;
    [[nodiscard]] bool get_status() const;
    [[nodiscard]] std::string get_img() const;
};

#endif // CONTAINER_HPP
