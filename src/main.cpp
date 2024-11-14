#include <sys/wait.h>
#include "mydocker.h"

int main(int argc, char* argv[]) {
    std::string id = "2";
    std::string no_procs = "5";
    Container cntnr = Container(id, false, no_procs);
    cntnr.run();
    return 0;
}
