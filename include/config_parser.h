#ifndef LAB7_SERIAL_COUNTING_CONFIG_PARSER_H
#define LAB7_SERIAL_COUNTING_CONFIG_PARSER_H

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

struct cnt_config {
public:
    std::string id;
    std::string n_pr;
    std::string max_memory;
    std::string lclfld;
    std::string image;

    explicit cnt_config(const std::string& cfg_file_name, ssize_t &status);
};

#endif //LAB7_SERIAL_COUNTING_CONFIG_PARSER_H
