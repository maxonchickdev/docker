#include "container.h"
#include <filesystem>
#include <unistd.h>
#include <cerrno>
#include <cstring>


namespace fs = std::filesystem;
void Container::set_permissions(const std::string& path) {
    if (chown(path.c_str(), 1000, 1000) == -1) {
        std::cerr << "Failed to change ownership of a directory." << strerror(errno) << '\n';
        exit(1);
    }
}

std::string Container::create_system(const std::string& source_path) {
    if (!fs::exists(source_path) || !fs::is_directory(source_path)) {
        std::cerr << "Source path does not exist or is not a directory: " + source_path << '\n';
        exit(1);
    }

    std::string dest_path = std::string(MNT_PATH) + "/" + containerID;
    if (!fs::exists(dest_path)) {
        fs::create_directories(dest_path);
    }
    set_permissions(dest_path);

    for (const auto& entry : fs::recursive_directory_iterator(source_path)) {
        const auto& source_file_path = entry.path();
        std::string relative_path_str = entry.path().string();
        std::string base_path_str = source_path + "/";
        if (relative_path_str.find(base_path_str) == 0) {
            relative_path_str = relative_path_str.substr(base_path_str.length());
        }
        auto destination_file_path = fs::path(dest_path) / relative_path_str;
        if (fs::is_directory(source_file_path)) {
            fs::create_directories(destination_file_path);
            set_permissions(destination_file_path.string());
        } else if (fs::is_regular_file(source_file_path)) {
            fs::copy_file(source_file_path, destination_file_path, std::filesystem::copy_options::overwrite_existing);
            set_permissions(destination_file_path.string());
        }
    }
    return dest_path;
}

void Container::initialize() {
    sources_path = create_system(std::string(TEMPLATE_PATH));
    cgroup_path = setup_cgroup();
}

