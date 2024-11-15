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

void Container::set_max_pid(pid_t comm_pid) {
    std::string procs_file_path = std::string(CG_PATH) + "/" + containerID + "/cgroup.procs";
    std::string pids_max_file_path = std::string(CG_PATH) + "/" + containerID + "/pids.max";
    std::ofstream procs_fd(procs_file_path);
    if (procs_fd.is_open()) {
        procs_fd << comm_pid;
        procs_fd.close();
    } else {
        std::cerr << "Failed to open a cgroup procs file: " << strerror(errno) << '\n';
        exit(1);
    }

    std::ofstream pidsmax_fd(pids_max_file_path);
    if (pidsmax_fd.is_open()) {
        pidsmax_fd << num_of_procs;
        pidsmax_fd.close();
    } else {
        std::cerr << "Failed to set pids.max value: " << strerror(errno) << '\n';
        exit(1);
    }
}