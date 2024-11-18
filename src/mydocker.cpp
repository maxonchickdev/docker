#include "mydocker.h"
#include <boost/algorithm/string.hpp>

void MyDocker::start_server(int port) {
    struct sockaddr_in addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Error while creating socket" << std::endl;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error while binding" << std::endl;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Error while listening" << std::endl;
    }

    std::cout << "Server is listening on port " << port << std::endl;
}


void MyDocker::run_server() {
    static const int BUFFER_SIZE = 1024;
    struct sockaddr_in address{};
    char buff[BUFFER_SIZE];
    int len = sizeof(address);
    alive = true;
    while (alive) {
        std::cout << "Waiting for connection..." << std::endl;

        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&len)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "Client connected: " << client_socket << std::endl;

        while (true) {
            memset(buff, 0, BUFFER_SIZE);
            ssize_t bytes_read = read(client_socket, buff, BUFFER_SIZE);

            if (bytes_read == 0) {
                std::cout << "Client disconnected" << std::endl;
                break;
            }

            std::string command(buff);
            if (!command.empty() && command[command.length()-1] == '\n') {
                command = command.substr(0, command.length()-1);
            }

            process_command(command, client_socket);
        }
        close(client_socket);
    }
}

void MyDocker::process_command(const std::string& cmd, int client_socket) {
    std::vector<std::string> args;
    std::string image;
    boost::split(args, cmd, boost::is_any_of(" "));
    if (args[0] == "mydocker") {
        std::string response;
        int cmd_no = my_commands[args[1]];
        switch (cmd_no){
            case 1:
                response = "Starting container\n";
                send(client_socket, response.c_str(), response.length(), 0);
                run_container(args[2], client_socket);
                break;
            case 2:
                response = "Stopping container\n";
                send(client_socket, response.c_str(), response.length(), 0);
//                stop_container("CONTAINERID");
//            close(container_pipe[1]); implement
                break;
            case 3:
                list_containers();
                break;
            case 4:
                image = "alp_minifs";
                create_container(image);
                break;
            case 6:
                alive = false;
                break;
            default:
                response = "Wrong command\n";
                send(client_socket, response.c_str(), response.length(), 0);
                break;
        }
    } else if (running_containers > 0){
        int w = write(cntr_pipe[1], (cmd + "\n").c_str(), cmd.length() + 1);

        if (cmd == "exit") {
            close(cntr_pipe[1]);
            running_containers--; // ADD IN RN and stop
            std::string msg = "Container stopped.\n";
            send(client_socket, msg.c_str(), msg.length(), 0);
        }
    } else {
        std::string msg = "No running containers, check usage.\n";
        send(client_socket, msg.c_str(), msg.length(), 0);
    };

}

void MyDocker::run_container(const std::string& id, int client_socket) {
    if (!(containers.find(id) != containers.end())) {
        std::string msg = "No such container.\n";
        send(client_socket, msg.c_str(), msg.length(), 0);
        return;
    }

    if (pipe(cntr_pipe) == -1) {
        std::string msg = "Error while creating pipe.\n";
        send(client_socket, msg.c_str(), msg.length(), 0);
        return;
    }

    dup2(cntr_pipe[0], STDIN_FILENO);
    dup2(client_socket, STDOUT_FILENO);
    dup2(client_socket, STDERR_FILENO);

    Container cntnr = containers.at(id);
    std::cout << "Started container: " << cntnr.getID() << "\n";
    cntnr.run();
    running_containers++;
}

void MyDocker::create_container(std::string& image) {
    bool valid = false;
    for (std::string i : IMAGES) {
        if (i == image) {
            valid = true;
            break;
        }
    }
    if (valid) {
        std::string c_id = "2";
        std::string n_pr = "5";
        std::vector<std::string> lclfld{"/home/serhii/test", "/home/serhii/test2"};
        Container cntnr = Container(c_id, true, lclfld, n_pr, image);
        std::cout << "Created container with ID: " << c_id << "\n";
        containers.emplace(c_id, cntnr);
    } else {
        std::cout << "Select from images:\n";
        for (std::string i : IMAGES) {
            std::cout << i << " ";
        }
        std::cout << "\n";
    }
}

void MyDocker::list_containers() {
    std::cout << "Your containers: \n";
    for (auto& [id, container] : containers) {
        std::cout << "Container id: " << id << std::endl;
    }
}

