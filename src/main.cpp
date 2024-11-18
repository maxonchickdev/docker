#include "mydocker.h"

int main(int argc, char* argv[]) {
    MyDocker my_docker;
    int port = std::stoi(argv[1]);
    my_docker.start_server(port);
    my_docker.run_server();

}