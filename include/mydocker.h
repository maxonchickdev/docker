#ifndef DOCKER_HPP
#define DOCKER_HPP

#include <string>
#include <unordered_map>
#include "container.h"

class Docker {
private:
    std::string baseDirectory;
    std::unordered_map<std::string, Container> containers;

    void loadConfiguration();
    void saveConfiguration();
    std::string generateContainerID();

public:
    Docker(const std::string& baseDir = "/home/zahar_kohut/mydocker");
    ~Docker();

    void initialize();
    std::string createContainer();
    bool runContainer(const std::string& containerID);
    bool stopContainer(const std::string& containerID);
    bool deleteContainer(const std::string& containerID);

    // Utility functions
    void listContainers() const;
};

#endif // DOCKER_HPP
