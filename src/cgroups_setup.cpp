#include "cgroups_setup.h"
#include <fstream>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

void setup_cgroup(const std::string& cg_name) {
    std::string cg_path = CG_PATH + cg_name;

    if (mkdir(cg_path.c_str(), 0775) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a cgroup: " << strerror(errno) << '\n';
        exit(1);
    }
}

void set_max_pid(const std::string& cg_name, pid_t comm_pid, size_t num_of_procs) {
    std::string procs_file_path = CG_PATH + cg_name + "/cgroup.procs";
    std::string pids_max_file_path = CG_PATH + cg_name + "/pids.max";
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