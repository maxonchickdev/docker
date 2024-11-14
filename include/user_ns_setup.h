#ifndef DOCKER_FIRST_ATTEMPT_USER_NS_SETUP_H
#define DOCKER_FIRST_ATTEMPT_USER_NS_SETUP_H
#include <iostream>

void setup_user_ns(int user_id, pid_t comm_pid);

#endif //DOCKER_FIRST_ATTEMPT_USER_NS_SETUP_H
