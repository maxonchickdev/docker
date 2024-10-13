#include <iostream>
#include <sched.h>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <sys/mount.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>


#define STACK_SIZE (1024 * 1024)
#define CG_PATH "/sys/fs/cgroup"
#define MNT_PATH "/home/zahar_kohut"
#define MINIFS_PATH "/home/zahar_kohut/minifs"

 int command_process(void* arg) {
    int* p_fd = (int*)arg;
    if (close(p_fd[1]) == -1) {
        std::cerr << "Failed to close writing end of pipe in child!" << '\n';
        return 1;
    }

    char buf[4];
    if (read(p_fd[0], buf, 4) != 4) {
        std::cerr << "Failed reading from pipe!" << '\n';
        return 1;
    } else {
        std::cout << "Child starts working!" << '\n';
    }

    if (close(p_fd[0]) == -1) {
        std::cerr << "Failed to close reading end of pipe in child!" << '\n';
        return 1;
    }

    std::string mount_point = "/minifs";
    std::string mount_p_path = MNT_PATH + mount_point;

    if (mkdir(mount_p_path.c_str(), 0775) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a mount point directory: " << strerror(errno) << '\n';
        return 1;
    }

    std::string fs_path = MINIFS_PATH;
    if (mount(fs_path.c_str(), mount_p_path.c_str(), "ext4", MS_BIND, "") != 0) {
        std::cerr << "Failed to mount a filesystem: " << strerror(errno) << '\n';
        return 1;
    }

    if (chdir(mount_p_path.c_str()) != 0) {
        std::cerr << "Failed to enter a mount point: " << strerror(errno) << '\n';
        return 1;
    };

    std::string put_old = "put_old";
    if (mkdir(put_old.c_str(), 0777) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a put_old directory: " << strerror(errno) << '\n';
        return 1;
    }

    if (syscall(SYS_pivot_root, ".", put_old.c_str()) != 0) {
        std::cerr << "Failed to setup a new root: " << strerror(errno) << '\n';
        return 1;
    }

    if (chdir("/") != 0) {
        std::cerr << "Failed to enter new root: " << strerror(errno) << '\n';
        return 1;
    };

    if (mkdir("proc", 555) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a proc directory: " << strerror(errno) << '\n';
        return 1;
    }

    if (mount("proc", "/proc", "proc", 0, "") != 0) {
        std::cerr << "Failed to mount a proc filesystem: " << strerror(errno) << '\n';
        return 1;
    }

    if (umount2(put_old.c_str(), MNT_DETACH) != 0) {
        std::cerr << "Failed to mount a filesystem: " << strerror(errno) << '\n';
        return 1;
    }

     if (setgid(0) == -1) {
         std::cerr << "Failed to set gid: " << strerror(errno) << '\n';
         return 1;
     }

     if (setuid(0) == -1)
     {
         std::cerr << "Failed to set uid: " << strerror(errno) << '\n';
         return 1;
     }

    std::cout << "======sh======" << '\n';
    if (execlp("sh", "sh", nullptr) == -1) {
        std::cerr << "Failed to execute command: " << strerror(errno) << '\n';
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    void* stack;
    void* stackTop;
    stack = mmap(nullptr, STACK_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    if (stack == MAP_FAILED) {
        std::cerr << "Failed to allocate stack!" << '\n';
        return 1;
    }

    stackTop = static_cast<char*>(stack) + STACK_SIZE;

    int p_fd[2];
    if (pipe(p_fd) == -1) {
        std::cerr << "Failed to create pipe!" << '\n';
        return 1;
    }

    int clone_flags = SIGCHLD | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWUTS;
    pid_t chld_pid = clone(command_process, stackTop, clone_flags, p_fd);

    if (chld_pid < 0) {
        std::cerr << "Failed to clone process:" << strerror(errno) << '\n';
        return 1;
    }

    std::string cg_name = "/test";
    std::string cg_path = CG_PATH + cg_name;
    std::string procs_file_path = cg_path + "/cgroup.procs";
    std::string pids_max_file_path = cg_path + "/pids.max";
    size_t num_of_procs = 5;

    if (mkdir(cg_path.c_str(), 0775) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create a cgroup: " << strerror(errno) << '\n';
        return 1;
    }

    std::ofstream procs_fd(procs_file_path);
    if (procs_fd.is_open()) {
        procs_fd << chld_pid;
        procs_fd.close();
    } else {
        std::cerr << "Failed to open a cgroup procs file: " << strerror(errno) << '\n';
        return 1;
    }

    std::ofstream pidsmax_fd(pids_max_file_path);
    if (pidsmax_fd.is_open()) {
        pidsmax_fd << num_of_procs;
        pidsmax_fd.close();
    } else {
        std::cerr << "Failed to set pids.max value: " << strerror(errno) << '\n';
        return 1;
    }

    int user_id = 1000;

    std::ofstream uid_m_fd("/proc/" + std::to_string(chld_pid) + "/uid_map");
    if (uid_m_fd.is_open()) {
        uid_m_fd << "0 " + std::to_string(user_id) + " 1\n";
        uid_m_fd.close();
    } else {
        std::cerr << "Failed to open uid map file: " << strerror(errno) << '\n';
        return 1;
    }

    std::ofstream set_g_fd("/proc/" + std::to_string(chld_pid) + "/setgroups");
    if (set_g_fd.is_open()) {
        set_g_fd << "deny";
        set_g_fd.close();
    } else {
        std::cerr << "Failed to open setgroups file: " << strerror(errno) << '\n';
        return 1;
    }

    std::ofstream gid_m_fd("/proc/" + std::to_string(chld_pid) + "/gid_map");
    if (gid_m_fd.is_open()) {
        gid_m_fd << "0 " + std::to_string(user_id) + " 1\n";
        gid_m_fd.close();
    } else {
        std::cerr << "Failed to open uid map file: " << strerror(errno) << '\n';
        return 1;
    }

    if (close(p_fd[0]) == -1) {
        std::cerr << "Failed to close reading end of pipe in parent!" << '\n';
        return 1;
    }

    if (write(p_fd[1], "jojo", 4) != 4) {
        std::cerr << "Failed writing to pipe!" << '\n';
        return 1;
    } else {
        std::cout << "Setup finished!" << '\n';
    }

    if (close(p_fd[1]) == -1) {
        std::cerr << "Failed to close writing end of pipe in parent!" << '\n';
        return 1;
    }

    if (waitpid(chld_pid, nullptr, 0) == -1) {
        std::cerr << "Failed to to wait for child:" << strerror(errno) << '\n';
        return 1;
    }

    return 0;
}
