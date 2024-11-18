#ifndef MYDOCKER_HPP
#define MYDOCKER_HPP

#include "container.h"

inline std::map<std::string, int> my_commands = {
        {"run", 1},
        {"stop", 2},
        {"list", 3},
        {"create", 4},
        {"delete", 5},
        {"shutdown", 6}
};


class MyDocker {
private:
    int server_fd = -1;
    int cntr_pipe[2] = {-1, -1};
    bool alive = false;
    int running_containers = 0;
    std::string baseDirectory;
    std::unordered_map<std::string, Container> containers;
public:
    MyDocker() = default;
    void run_server();
    void start_server(int port);
    void process_command(const std::string& cmd, int client_socket);

    void create_container(std::string& cfg, bool is_new);
    void run_container(const std::string& containerID, int client_socket);
    void stop_container(const std::string& id);
    void list_containers();
    void delete_container(const std::string& id);
    ~MyDocker() = default;
};

#endif // MYDOCKER_HPP
