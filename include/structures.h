#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>

namespace fs = std::filesystem;

struct Object {
    std::string name;
    fs::file_time_type modification_time;
    uintmax_t size = 0;
};

struct AppState {
    fs::path current_path;
    std::vector<Object> files;
    int selected_index = 0, file_viewer_dummy = 0;
    int sort_type = 1;
    std::string error;
    int text_display = 0;
    std::vector<std::string> file_lines;
};


