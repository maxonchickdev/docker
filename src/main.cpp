#include "mydocker.h"

int main(int argc, char* argv[]) {
//    Container cnt("3", true, {}, "max", "max", "alp_minifs");
//    cnt.run();
    MyDocker my_docker;
    int port = std::stoi(argv[1]);
    my_docker.start_server(port);
    my_docker.run_server();
}