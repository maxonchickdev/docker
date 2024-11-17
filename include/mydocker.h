#ifndef MYDOCKER_HPP
#define MYDOCKER_HPP


#include "container.h"

inline std::map<std::string, int> my_commands = {
        {"run", 1},
        {"stop", 2}
};

class MyDocker {
private:
    int server_fd;
    int cntr_pipe[2];
    int running_containers;
    std::string baseDirectory;
    std::unordered_map<std::string, Container> containers;

public:
    MyDocker() = default;
//    MyDocker(const std::string& baseDir = MNT_PATH);
//    void initialize();
    void start_server(int port);

    void run_server();
    void process_command(const std::string& cmd, int client_socket);

    std::string create_container();
    void run_container(const std::string& containerID, int client_socket);
//    void stop_container(const std::string& containerID);
    void delete_container(const std::string& containerID);
    void list_containers() const;

    ~MyDocker() {
        close(server_fd);
    }

};

#endif // MYDOCKER_HPP
