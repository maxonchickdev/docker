#include <cgroups_setup.h>
#include <user_ns_setup.h>
#include <mount_ns_setup.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>

#define STACK_SIZE (1024 * 1024)

void* create_stack() {
    void* stack;
    void* stack_top;
    stack = mmap(nullptr, STACK_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    if (stack == MAP_FAILED) {
        std::cerr << "Failed to allocate stack!" << '\n';
    }

    stack_top = static_cast<char*>(stack) + STACK_SIZE;
    return stack_top;
}

int command_process(void* arg) {
    int* p_fd = (int*)arg;
    if (close(p_fd[1]) == -1) {
        std::cerr << "Failed to close writing end of pipe in child!" << '\n';
        exit(1);
    }

    char buf[4];
    if (read(p_fd[0], buf, 4) != 4) {
        std::cerr << "Failed reading from pipe!" << '\n';
        exit(1);
    } else {
        std::cout << "Child starts working!" << '\n';
    }

    if (close(p_fd[0]) == -1) {
        std::cerr << "Failed to close reading end of pipe in child!" << '\n';
        exit(1);
    }

    std::string mount_point = "/minifs";
    setup_mount_ns(mount_point);

    if (setgid(0) == -1) {
        std::cerr << "Failed to set gid: " << strerror(errno) << '\n';
        exit(1);
    }

    if (setuid(0) == -1) {
        std::cerr << "Failed to set uid: " << strerror(errno) << '\n';
        exit(1);
    }

    if (execlp("sh", "sh", nullptr) == -1) {
        std::cerr << "Failed to execute command: " << strerror(errno) << '\n';
        exit(1);
    }
    return 0;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Wrong number of arguments!" << '\n';
        return 1;
    }

    int p_fd[2];
    if (pipe(p_fd) == -1) {
        std::cerr << "Failed to create pipe!" << '\n';
        return 1;
    }

    auto stack = create_stack();
    int clone_flags = SIGCHLD | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWUTS;
    pid_t comm_pid = clone(command_process, stack, clone_flags, p_fd);

    if (comm_pid < 0) {
        std::cerr << "Failed to clone process:" << strerror(errno) << '\n';
        return 1;
    }

    std::string cg_name = "/test2";
    size_t num_of_procs = std::stoi(argv[1]);
    setup_cgroup(cg_name);
    set_max_pid(cg_name, comm_pid, num_of_procs);

    int user_id = 1000;
    setup_user_ns(user_id, comm_pid);

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

    if (waitpid(comm_pid, nullptr, 0) == -1) {
        std::cerr << "Failed to to wait for child:" << strerror(errno) << '\n';
        return 1;
    }

    return 0;
}
