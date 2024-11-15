#include <string>
#include "container.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <utility>


void* Container::create_stack() {
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

int Container::command_process(void* arg) {
    auto ref = static_cast<Container *>(arg);
    if (close(ref->p_fd[1]) == -1) {
        std::cerr << "Failed to close writing end of pipe in child!" << '\n';
        exit(1);
    }

    char buf[4];
    if (read(ref->p_fd[0], buf, 4) != 4) {
        std::cerr << "Failed reading from pipe!" << '\n';
        exit(1);
    } else {
        std::cout << "Child starts working!" << '\n';
    }

    if (close(ref->p_fd[0]) == -1) {
        std::cerr << "Failed to close reading end of pipe in child!" << '\n';
        exit(1);
    }

    ref->setup_mount_ns();

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

int Container::spawn_process(int (*commproc)(void *), void *arg_for_commproc) {
    auto stack = create_stack();
    int clone_flags = SIGCHLD | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWUTS;
    return clone(commproc, stack, clone_flags, arg_for_commproc);
}


Container::Container(std::string id, bool is_new, std::string num_of_procs): containerID(std::move(id)), num_of_procs(std::move(num_of_procs)), isRunning(false) {
    if (pipe(p_fd) == -1) {
        std::cerr << "Failed to create pipe!" << '\n';
        exit(1);
    }
    if (is_new) {
        initialize();
    } else {
        sources_path = std::string(MNT_PATH) + "/" + containerID;
        cgroup_path = std::string(CG_PATH) + "/" + containerID;
    }
}

void Container::run() {
    pid_t comm_pid = spawn_process(&Container::command_process, this);

    if (comm_pid < 0) {
        std::cerr << "Failed to clone process:" << strerror(errno) << '\n';
        exit(1);
    }
    set_max_pid(comm_pid);
    int user_id = 1000;
    setup_user_ns(user_id, comm_pid);

    if (close(p_fd[0]) == -1) {
        std::cerr << "Failed to close reading end of pipe in parent!" << '\n';
        exit(1);
    }

    if (write(p_fd[1], "jojo", 4) != 4) {
        std::cerr << "Failed writing to pipe!" << '\n';
        exit(1);
    } else {
        std::cout << "Setup finished!" << '\n';
    }

    if (close(p_fd[1]) == -1) {
        std::cerr << "Failed to close writing end of pipe in parent!" << '\n';
        exit(1);
    }

    if (waitpid(comm_pid, nullptr, 0) == -1) {
        std::cerr << "Failed to to wait for child:" << strerror(errno) << '\n';
        exit(1);
    }
    exit(0);
}
void stop();
void deleteResources();

std::string Container::getID() const {
    return containerID;
}
bool Container::isContainerRunning() const {
    return isRunning;
}
