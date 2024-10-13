#include "mount_ns_setup.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <syscall.h>
#include <cerrno>
#include <cstring>

void setup_mount_ns(const std::string& mount_point) {
    std::string mount_p_path = MNT_PATH + mount_point;

    if (mkdir(mount_p_path.c_str(), 0775) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a mount point directory: " << strerror(errno) << '\n';
        exit(1);
    }

    std::string fs_path = MINIFS_PATH;
    if (mount(fs_path.c_str(), mount_p_path.c_str(), "ext4", MS_BIND, "") != 0) {
        std::cerr << "Failed to mount a filesystem: " << strerror(errno) << '\n';
        exit(1);
    }

    if (chdir(mount_p_path.c_str()) != 0) {
        std::cerr << "Failed to enter a mount point: " << strerror(errno) << '\n';
        exit(1);
    };

    std::string put_old = "put_old";
    if (mkdir(put_old.c_str(), 0777) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a put_old directory: " << strerror(errno) << '\n';
        exit(1);
    }

    if (syscall(SYS_pivot_root, ".", put_old.c_str()) != 0) {
        std::cerr << "Failed to setup a new root: " << strerror(errno) << '\n';
        exit(1);
    }

    if (chdir("/") != 0) {
        std::cerr << "Failed to enter new root: " << strerror(errno) << '\n';
        exit(1);
    };

    if (mkdir("proc", 555) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a proc directory: " << strerror(errno) << '\n';
        exit(1);
    }

    if (mount("proc", "/proc", "proc", 0, "") != 0) {
        std::cerr << "Failed to mount a proc filesystem: " << strerror(errno) << '\n';
        exit(1);
    }

    if (umount2(put_old.c_str(), MNT_DETACH) != 0) {
        std::cerr << "Failed to mount a filesystem: " << strerror(errno) << '\n';
        exit(1);
    }
}