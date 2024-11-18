#include "config_parser.h"

cnt_config::cnt_config(const std::string& cfg_file_name, ssize_t& status) {
    boost::program_options::options_description desc("Options");
    boost::program_options::variables_map vm;

    desc.add_options()
            ("id", boost::program_options::value<std::string>(), "container id")
            ("n_pr", boost::program_options::value<std::string>(), "max number of processes")
            ("max_memory", boost::program_options::value<std::string>(), "max memory in bytes")
            ("lclfld", boost::program_options::value<std::string>(), "list of folders to mount")
            ("image", boost::program_options::value<std::string>(), "image");


    std::ifstream cfg_file(cfg_file_name);

    if (!cfg_file.is_open()) {
        std::cerr << "Error opening config file" << '\n';
        status = 1;
        return;
    }

    try {
        boost::program_options::store(boost::program_options::parse_config_file(cfg_file, desc), vm);
        boost::program_options::notify(vm);
    } catch (const boost::program_options::error& e) {
        status = 1;
        std::cerr << "Error reading config" << '\n';
    }

    id = vm["id"].as<std::string>();
    n_pr = vm["n_pr"].as<std::string>();
    max_memory = vm["max_memory"].as<std::string>();
    lclfld = vm["lclfld"].as<std::string>();
    image = vm["image"].as<std::string>();
    status = 0;
}