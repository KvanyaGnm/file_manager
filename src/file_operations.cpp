#include "file_operations.h"
#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>

void refresh_files(AppState& state) {
    state.files.clear();
    state.error.clear();
    state.files.push_back({"..", });

    std::vector<Object> dirs, files;
    try {
        for (const auto& entry : fs::directory_iterator(state.current_path)) {
            auto name = entry.path().filename().string();
            auto modif_time = fs::last_write_time(entry.path());
            if (name.empty()){
                continue;
            }
            if (entry.is_directory()){
                dirs.push_back({name, modif_time});
            } 
            else {
                files.push_back({name, modif_time});
            }
        }
    } catch (const std::exception& e) {
        state.error = std::string(e.what());
    }
    int sort_mode = std::abs(state.sort_type);
    bool rev = state.sort_type < 0;
    auto cmp = [sort_mode, rev](const Object& a, const Object& b) {
        bool smaller = false;
        switch(sort_mode){
            case 1:
                smaller = a.name < b.name;
                break;
            case 2:
                smaller = a.modification_time < b.modification_time;
                break;
        }
        return rev ? !smaller : smaller;
    };

    std::sort(dirs.begin(), dirs.end(), cmp);
    std::sort(files.begin(), files.end(), cmp);

    state.files.insert(state.files.end(), dirs.begin(), dirs.end());
    state.files.insert(state.files.end(), files.begin(), files.end());
    state.selected_index = 0;
}
void update_file_info(AppState& state, std::vector<std::string>& filenames, std::vector<std::string>& datetime){
    filenames.clear();
    datetime.clear();
    for (auto& x : state.files){
        filenames.push_back(std::move(x.name));

        if (x.name != ".."){
            auto only_seconds = std::chrono::floor<std::chrono::seconds>(x.modification_time);
            datetime.push_back(std::format("{:%Y-%m-%d %H:%M:%S}", only_seconds));
        }else{
            datetime.push_back("");
        }
    }
}

void read_file(const fs::path& filepath, std::vector<std::string>& file_lines){
    file_lines.clear();
    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line)) {
        file_lines.push_back(line);
    }
}


