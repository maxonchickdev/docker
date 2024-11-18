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
    void start_server(int port);
    void run_server();
    void process_command(const std::string& cmd, int client_socket);

    void create_container(std::string& image);

    void run_container(const std::string& containerID, int client_socket);
//    void stop_container(const std::string& containerID);
//    void delete_container(const std::string& containerID);
    void list_containers();

    ~MyDocker() {
        close(server_fd);
    }

};

#endif // MYDOCKER_HPP
