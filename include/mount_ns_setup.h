#ifndef DOCKER_FIRST_ATTEMPT_MOUNT_NS_SETUP_H
#define DOCKER_FIRST_ATTEMPT_MOUNT_NS_SETUP_H

#include <iostream>

#define MNT_PATH "/home/zahar_kohut"
#define MINIFS_PATH "/home/zahar_kohut/minifs"

void setup_mount_ns(const std::string& mount_point);

#endif //DOCKER_FIRST_ATTEMPT_MOUNT_NS_SETUP_H
