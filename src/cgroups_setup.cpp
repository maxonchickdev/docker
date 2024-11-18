#include "container.h"
#include <fstream>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>


std::string Container::setup_cgroup() {
    std::string cg_path = std::string(CG_PATH) + "/" + containerID;

    if (mkdir(cg_path.c_str(), 0775) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a cgroup: " << strerror(errno) << '\n';
        exit(1);
    }
    return cg_path;
}

void Container::add_to_cgroup(pid_t comm_pid) {
    std::string procs_file_path = std::string(CG_PATH) + "/" + containerID + "/cgroup.procs";
    std::ofstream procs_fd(procs_file_path);
    if (procs_fd.is_open()) {
        procs_fd << comm_pid;
        procs_fd.close();
    } else {
        std::cerr << "Failed to open a cgroup procs file: " << strerror(errno) << '\n';
        exit(1);
    }
}

void Container::set_max_pid() {
    std::string pids_max_file_path = std::string(CG_PATH) + "/" + containerID + "/pids.max";
    std::ofstream pidsmax_fd(pids_max_file_path);
    if (pidsmax_fd.is_open()) {
        pidsmax_fd << num_of_procs;
        pidsmax_fd.close();
    } else {
        std::cerr << "Failed to set pids.max value: " << strerror(errno) << '\n';
        exit(1);
    }
}

void Container::set_max_memory() {
    std::string memory_max_file_path = std::string(CG_PATH) + "/" + containerID + "/memory.max";
    std::ofstream memmax_fd(memory_max_file_path);
    if (memmax_fd.is_open()) {
        memmax_fd << max_memory;
        memmax_fd.close();
    } else {
        std::cerr << "Failed to set pids.max value: " << strerror(errno) << '\n';
        exit(1);
    }
}