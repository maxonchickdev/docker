#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <string>
#include <iostream>
#define STACK_SIZE (1024 * 1024)
#define CG_PATH "/sys/fs/cgroup"


class Container {
private:
    int p_fd[2];
    std::string containerID;
    std::string cgroup_path;
    std::string sources_path;
    std::string num_of_procs;
    bool isRunning;

    static void* create_stack();
    std::string setup_cgroup();
    static void setup_user_ns(int user_id, pid_t comm_pid);
    void set_max_pid(pid_t comm_pid);
    static int command_process(void* arg);
    static int spawn_process(int (*child_func)(void *), void *arg_for_commproc);
    static void set_permissions(const std::string& path);
    void setup_mount_ns();
    std::string create_system(const std::string& source_path);


public:
    Container(std::string id, bool is_new, std::string num_of_procs = "max");
    void initialize();
    void run();
    void stop();
    void deleteResources();

    std::string getID() const;
    bool isContainerRunning() const;
};

#endif // CONTAINER_HPP
