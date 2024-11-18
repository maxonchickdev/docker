#include "mydocker.h"
#include <boost/algorithm/string.hpp>
#include "config_parser.h"
#include <filesystem>

void MyDocker::start_server(int port) {
    struct sockaddr_in addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Error while creating socket" << std::endl;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    std::string meta = std::string(TEMPLATE_PATH) + "/meta";
    for (const auto& entry : std::filesystem::directory_iterator(meta)) {
        if (std::filesystem::is_regular_file(entry.status())) {
            std::string cfg = entry.path().string();
            create_container(cfg, false);
        }
    }

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

        while (alive) {
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
    std::string cfg;
    boost::split(args, cmd, boost::is_any_of(" "));
    if (args[0] == "mydocker" && args.size() <= 3) {
        std::string response;
        int cmd_no = my_commands[args[1]];
        if (args.size() == 2 && (cmd_no != 3 && cmd_no != 6 && cmd_no != 0)) {
            //std::cout << "cmd_no " <<cmd_no <<"\n";
            if (cmd_no == 4) {
                std::cout << "provide config path id " <<"\n";
            } else {
                std::cout << "select container id " <<"\n";
            }
            return;
        }
        else if (args.size() == 3 && args[2].empty() && (cmd_no != 3 && cmd_no != 6 && cmd_no != 0)) {
            //std::cout << "cmd_no " <<cmd_no <<"\n";
            if (cmd_no == 4) {
                std::cout << "provide config path " <<"\n";
            } else {
                std::cout << "select container id " <<"\n";
            }
            return;
        }
        switch (cmd_no){
            case 1:
                response = "Starting container\n";
                send(client_socket, response.c_str(), response.length(), 0);
                run_container(args[2], client_socket);
                break;
            case 2:
                response = "Stopping container\n";
                send(client_socket, response.c_str(), response.length(), 0);
                stop_container(args[2]);
                break;
            case 3:
                list_containers();
                break;
            case 4:
                cfg = args[2];
                create_container(cfg, true);
                break;
            case 5:
                response = "Deleting container\n";
                send(client_socket, response.c_str(), response.length(), 0);
                delete_container(args[2]);
                break;
            case 6:
                response = "Server is shutting down\n";
                send(client_socket, response.c_str(), response.length(), 0);
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

void MyDocker::create_container(std::string& cfg, bool is_new) {
    ssize_t status;
    std::string image;
    std::string n_pr;
    std::string max_m;
    cnt_config cnt_config(cfg, status);
    if (status != 0) {
        std::cout << "Reading from config failed.\n";
        return;
    }
    if (containers.find(cnt_config.id) != containers.end()) {
        std::cout << "Container with such ID already exists.\n";
        return;
    }
    std::string meta = std::string(TEMPLATE_PATH) + "/meta";
    if (is_new) {
        std::filesystem::path dest = std::filesystem::path(meta) / std::filesystem::path(cnt_config.id + ".cfg");
        std::filesystem::copy(cfg, dest, std::filesystem::copy_options::overwrite_existing);
    }
    if (cnt_config.id.empty() || cnt_config.image.empty()) {
        std::cout << "Invalid config, image or id is not set.\n";
        return;
    }
    image = cnt_config.image;
    bool valid = false;
    for (std::string i : IMAGES) {
        if (i == image) {
            valid = true;
            break;
        }
    }
    if (valid) {
        std::vector<std::string> lclfld {};
        if (!cnt_config.lclfld.empty()) {
            std::stringstream ss(cnt_config.lclfld);
            std::string item;

            while (std::getline(ss, item, ',')) {
                lclfld.push_back(item);
            }
        }
        if (cnt_config.n_pr.empty()) {
            n_pr = "max";
        } else {
            n_pr = cnt_config.n_pr;
        }
        if (cnt_config.max_memory.empty()) {
            max_m = "max";
        } else {
            max_m = cnt_config.max_memory;
        }
        std::cout << "Created container with ID: " << cnt_config.id << "\n";
        containers.emplace(cnt_config.id, Container(cnt_config.id, is_new, lclfld, n_pr, max_m, image));
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

void MyDocker::stop_container(const std::string& id) {
    Container cntnr = containers.at(id);
    cntnr.stop();
    std::cout << "Stopped container: " << cntnr.getID() << "\n";
    running_containers--;
    close(cntr_pipe[1]);
}

void MyDocker::delete_container(const std::string& id) {
    Container cntnr = containers.at(id);
    std::string cfg = std::string(TEMPLATE_PATH) + "/meta/" + id + ".cfg";
    if (std::remove(cfg.c_str()) != 0) {
        std::cout << "Failed to remove config file.\n";
    }
    //stop_container(id);
    cntnr.remove();
    std::cout << "Deleted container: " << cntnr.getID() << "\n";
    containers.erase(id);
    // remove from json with containers
}