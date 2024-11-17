////#include <sys/wait.h>
////#include "container.h"
////
////#include <thread>
////#include <chrono>
////
////void stop_after_delay(Container& container, int seconds) {
////    std::this_thread::sleep_for(std::chrono::seconds(seconds));
////    std::cout << "Stopping container after " << seconds << " seconds...\n";
////    container.stop();
////}
////
////int main(int argc, char* argv[]) {
////    std::string id = "1";
////    std::string no_procs = "5";
////    Container cntnr = Container(id, true, no_procs);
////    cntnr.run();
////    std::thread stop_thread(stop_after_delay, std::ref(cntnr), 5);
////
////    stop_thread.join();
////    cntnr.run();
////
////    std::cout << "imwa";
////    return 0;
////}
////
//
//#include <iostream>
//#include <string>
//#include <sstream>
//#include <cstring>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include "container.h"
//#include <sys/wait.h>
//#include <sched.h>
//#include "vector"
//#include <fstream> // Required for std::ifstream
//
//class CommandServer {
//private:
//    int server_fd;
//    int port;
//    bool running_container = false;
//    static const int BUFFER_SIZE = 1024;
//    pid_t container_pid = -1;     // Process ID of the running container (-1 means no container)
//    int container_pipe[2] = {-1, -1};  // Pipe for communication with container
//
//    void runContainer(int client_socket) {
//        // Create communication pipe between server and container
//        if (pipe(container_pipe) == -1) {
//            throw std::runtime_error("Failed to create pipe");
//        }
//
//        // Fork a new process for the container
////        container_pid = fork();
////        if (container_pid == -1) {
////            throw std::runtime_error("Failed to fork container process");
////        }
//
////        if (container_pid == 0) {
//        running_container = true;
//        if (running_container) {
////            close(container_pipe[1]);
////            popen(container_pipe[0]);
////            close(container_pipe[0]);  // Close read end of pipe
//
//            dup2(container_pipe[0], STDIN_FILENO);
//            dup2(client_socket, STDOUT_FILENO);
//            dup2(client_socket, STDERR_FILENO);
//
//            try {
//                std::string c_id = "4";
//                std::string n_pr = "5";
//                Container cntnr = Container(c_id, true, n_pr);
//                cntnr.run();
////                container_pid = std::stoi(cntnr.getID());
//                container_pid = cntnr.miwa_suka();
////                cntnr.
//                std::cout << "started: " << container_pid << "\n";
//
////                setupContainer();  // Set up container isolation
//
////                // Change root directory to container directory
////                chroot("/container");
////                chdir("/");
////
////                // Start shell inside container
////                execl("/bin/sh", "sh", nullptr);
////                exit(1);
//            } catch (const std::exception& e) {
//                std::cerr << "Container setup failed: " << e.what() << std::endl;
////                exit(1);
//            }
//        } else {  // Parent process (server)
//            close(container_pipe[0]);  // Close read end of pipe
//            std::string welcome = "Container started. Type commands or 'exit' to quit.\n";
//            send(client_socket, welcome.c_str(), welcome.length(), 0);
//        }
//    }
//
//
//    void killProcessesInCgroup(const std::string& cgroupProcsPath) {
//        std::ifstream file(cgroupProcsPath);
//        if (!file.is_open()) {
//            throw std::runtime_error("Failed to open " + cgroupProcsPath);
//        }
//
//        std::vector<int> pids;
//        std::string line;
//
//        while (std::getline(file, line)) {
//            try {
//                int pid = std::stoi(line);
//                pids.push_back(pid);
//            } catch (const std::invalid_argument& e) {
//                std::cerr << "Invalid PID found in " << cgroupProcsPath << ": " << line << std::endl;
//            }
//        }
//
//        file.close();
//
//        for (int pid : pids) {
//            if (kill(pid, SIGKILL) == 0) {
//                std::cout << "Successfully killed process with PID: " << pid << std::endl;
//            } else {
//                perror(("Failed to kill process with PID: " + std::to_string(pid)).c_str());
//            }
//        }
//    }
//
//
//    void processCommand(const std::string& command, int client_socket) {
//        if (command.substr(0, 13) == "mydocker run ") {
//            // Handle container creation command
//            std::string template_name = command.substr(13);
//            std::string response = "Starting container with template: " + template_name + "\n";
//            send(client_socket, response.c_str(), response.length(), 0);
//
//            try {
//                runContainer(client_socket);
//            } catch (const std::exception& e) {
//                std::string error = "Failed to start container: " + std::string(e.what()) + "\n";
//                send(client_socket, error.c_str(), error.length(), 0);
//                return;
//            }
//        }
//        else if (command.substr(0, 4) == "miwa") {
//            std::cout << "1\n";
//            std::system("cat /sys/fs/cgroup/4/cgroup.procs");
//            killProcessesInCgroup("/sys/fs/cgroup/4/cgroup.procs");
//            std::cout << "2\n";
//            std::system("cat /sys/fs/cgroup/4/cgroup.procs");
//            container_pid = -1;
//            close(container_pipe[1]);
//            std::string msg = "Container stopped.\n";
//            send(client_socket, msg.c_str(), msg.length(), 0);
//        }
//        else if (container_pid > 0) {
//            // If container is running, send command to it
//            write(container_pipe[1], (command + "\n").c_str(), command.length() + 1);
//
//            if (command == "exit") {
//                // Handle container exit
//                int status;
////                waitpid(container_pid, &status, 0);  // Wait for container to finish
//                container_pid = -1;
//                close(container_pipe[1]);
//                std::string msg = "Container stopped.\n";
//                send(client_socket, msg.c_str(), msg.length(), 0);
//            }
//        }
//        else {
//            // No container running
//            std::string response = "No container running. Use 'mydocker run <template>' to start one.\n";
//            send(client_socket, response.c_str(), response.length(), 0);
//        }
//    }
//
//
//
//
//public:
//    CommandServer(int port) : port(port) {}
//
//    bool start() {
//        struct sockaddr_in address;
//        int opt = 1;
//
//        // Create TCP socket
//        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//            std::cerr << "Socket creation failed" << std::endl;
//            return false;
//        }
//
//        // Set socket options (reuse address and port)
//        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//            std::cerr << "Setsockopt failed" << std::endl;
//            return false;
//        }
//
//        // Set up address structure
//        address.sin_family = AF_INET;
//        address.sin_addr.s_addr = INADDR_ANY;
//        address.sin_port = htons(port);
//
//        // Bind socket to address
//        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
//            std::cerr << "Bind failed" << std::endl;
//            return false;
//        }
//
//        // Start listening for connections
//        if (listen(server_fd, 3) < 0) {
//            std::cerr << "Listen failed" << std::endl;
//            return false;
//        }
//
//        std::cout << "Server is listening on port " << port << std::endl;
//        return true;
//    }
//
//
//    // Main server loop
//    void run() {
//        struct sockaddr_in address;
//        int addrlen = sizeof(address);
//        char buffer[BUFFER_SIZE] = {0};
////        std::vector<int> sockets;
//
//        while (true) {
//            std::cout << "Waiting for connection..." << std::endl;
//
//            // Accept new client connection
//            int client_socket;
//            if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
//                std::cerr << "Accept failed" << std::endl;
//                continue;
//            }
//            std::cout << client_socket << "\n";
//
//            std::cout << "Client connected" << std::endl;
//
//            // Handle client commands
//            while (true) {
//                memset(buffer, 0, BUFFER_SIZE);
//                int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
//
//                if (bytes_read <= 0) {
//                    // Client disconnected - clean up container
//                    std::cout << "Client disconnected" << std::endl;
//                    if (container_pid > 0) {
////                        kill(container_pid, SIGTERM);
////                        waitpid(container_pid, nullptr, 0);
//                        container_pid = -1;
//                        close(container_pipe[1]);
//                    }
//                    break;
//                }
//
//                // Process received command
//                std::string command(buffer);
//                if (!command.empty() && command[command.length()-1] == '\n') {
//                    command = command.substr(0, command.length()-1);
//                }
//
//                processCommand(command, client_socket);
//            }
//            close(client_socket);
//
//        }
//    }
//
////
////    void run() {
////        struct sockaddr_in address;
////        int addrlen = sizeof(address);
////        char buffer[BUFFER_SIZE] = {0};
////
////        while (true) {
////            std::cout << "Waiting for new connection..." << std::endl;
////
////            int client_socket;
////            if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
////                std::cerr << "Accept failed" << std::endl;
////                continue;
////            }
////
////            std::cout << "Client connected" << std::endl;
////
////            //
////            while (true) {
////                memset(buffer, 0, BUFFER_SIZE);
////                int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
////
////                if (bytes_read <= 0) {
////                    std::cout << "Client disconnected" << std::endl;
////                    break;
////                }
////
////                // Remove newline character if present
////                std::string command(buffer);
////                if (!command.empty() && command[command.length()-1] == '\n') {
////                    command = command.substr(0, command.length()-1);
////                }
////
////                processCommand(command, client_socket);
////
////                if (command == "quit") {
////                    break;
////                }
////            }
////
////            close(client_socket);
////        }
////    }
//
//    // Destructor - cleanup
//    ~CommandServer() {
//        if (container_pid > 0) {
//            kill(container_pid, SIGTERM);  // Kill container if still running
//            waitpid(container_pid, nullptr, 0);
//        }
//        if (container_pipe[1] != -1) {
//            close(container_pipe[1]);  // Close pipe
//        }
//        close(server_fd);  // Close server socket
//    }
//};
//
//int main(int argc, char* argv[]) {
//
//
//    if (argc != 2) {
//        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
//        return 1;
//    }
//
//    int port = std::stoi(argv[1]);
////    std::cout << port;
//    CommandServer server(port);
//
//    if (!server.start()) {
//        std::cerr << "Failed to start server" << std::endl;
//        return 1;
//    }
//
//    server.run();
//    return 0;
//}


#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "container.h"
#include <sys/wait.h>
#include <sched.h>
#include "vector"
#include <fstream> // Required for std::ifstream
#include "mydocker.h"

int main(int argc, char* argv[]) {
    MyDocker my_docker;
    int port = std::stoi(argv[1]);
    my_docker.start_server(port);
    my_docker.run_server();

}