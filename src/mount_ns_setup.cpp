#include "container.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <syscall.h>
#include <cerrno>
#include <cstring>

void Container::setup_mount_ns() {

    if (mkdir(sources_path.c_str(), 0775) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a mount point directory: " << strerror(errno) << '\n';
        exit(1);
    }

    if (mount(sources_path.c_str(), sources_path.c_str(), "ext4", MS_BIND, "") != 0) {
        std::cerr << "Failed to mount a filesystem: " << strerror(errno) << '\n';
        exit(1);
    }

    if (chdir(sources_path.c_str()) != 0) {
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

    for (const auto& path : local_folders) {
        std::string folder_name = path.substr(path.find_last_of('/') + 1);
        if (mkdir(folder_name.c_str(), 0775) == -1 && errno != EEXIST) {
            std::cerr << "Failed to create directory for " << folder_name << ": " << strerror(errno) << '\n';
            continue;
        }
        std::string old_path = "/put_old" + path;
        if (mount(old_path.c_str(), folder_name.c_str(), "ext4", MS_BIND, "") != 0) {
            std::cerr << "Failed to mount " << old_path << " to " << folder_name << ": " << strerror(errno) << '\n';
            rmdir(folder_name.c_str());
            continue;
        }
    }

    if (umount2(put_old.c_str(), MNT_DETACH) != 0) {
        std::cerr << "Failed to mount a filesystem: " << strerror(errno) << '\n';
        exit(1);
    }
}