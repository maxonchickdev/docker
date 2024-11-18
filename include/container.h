#ifndef CONTAINER_HPP
#define CONTAINER_HPP

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


#define STACK_SIZE (1024 * 1024)
#define CG_PATH "/sys/fs/cgroup"
#define IMAGES {"alp_minifs", "alp_minifs_extend"}

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

    static void* create_stack();
    std::string setup_cgroup();
    static void setup_user_ns(int user_id, pid_t comm_pid);
    void add_to_cgroup(pid_t comm_pid);
    void set_max_pid();
    void set_max_memory();
    static int command_process(void* arg);
    static int spawn_process(int (*child_func)(void *), void *arg_for_commproc);
    static void set_permissions(const std::string& path);
    void setup_mount_ns();
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
};

#endif // CONTAINER_HPP
