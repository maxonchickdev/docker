#include <sys/wait.h>
#include "mydocker.h"

int main(int argc, char* argv[]) {
    std::string id = "1";
    std::string no_procs = "5";
    std::vector<std::string> lclfld{"/home/zahar_kohut/te", "/home/zahar_kohut/test2"};
    Container cntnr = Container(id, true, lclfld, no_procs);
    cntnr.run();
    return 0;
}
