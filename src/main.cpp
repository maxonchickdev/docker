#include <sys/wait.h>
#include "mydocker.h"

int main(int argc, char* argv[]) {
    std::string id = "1";
    std::string no_procs = "5";
    Container cntnr = Container(id, true, no_procs);
    cntnr.run();
    return 0;
}
