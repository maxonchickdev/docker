#include "user_ns_setup.h"
#include <fstream>
#include <cerrno>
#include <cstring>

void setup_user_ns(int user_id, pid_t comm_pid) {
    std::ofstream uid_m_fd("/proc/" + std::to_string(comm_pid) + "/uid_map");
    if (uid_m_fd.is_open()) {
        uid_m_fd << "0 " + std::to_string(user_id) + " 1\n";
        uid_m_fd.close();
    } else {
        std::cerr << "Failed to open uid map file: " << strerror(errno) << '\n';
        exit(1);
    }

    std::ofstream set_g_fd("/proc/" + std::to_string(comm_pid) + "/setgroups");
    if (set_g_fd.is_open()) {
        set_g_fd << "deny";
        set_g_fd.close();
    } else {
        std::cerr << "Failed to open setgroups file: " << strerror(errno) << '\n';
        exit(1);
    }

    std::ofstream gid_m_fd("/proc/" + std::to_string(comm_pid) + "/gid_map");
    if (gid_m_fd.is_open()) {
        gid_m_fd << "0 " + std::to_string(user_id) + " 1\n";
        gid_m_fd.close();
    } else {
        std::cerr << "Failed to open uid map file: " << strerror(errno) << '\n';
        exit(1);
    }
}