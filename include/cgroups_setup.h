#ifndef DOCKER_FIRST_ATTEMPT_CGROUPS_SETUP_H
#define DOCKER_FIRST_ATTEMPT_CGROUPS_SETUP_H

#include <iostream>

#define CG_PATH "/sys/fs/cgroup"


void setup_cgroup(const std::string& cg_name);
void set_max_pid(const std::string& cg_name, pid_t comm_pid, size_t num_of_procs);

#endif //DOCKER_FIRST_ATTEMPT_CGROUPS_SETUP_H
