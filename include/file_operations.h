#pragma once
#include "structures.h"
#include <vector>
#include <string>



void refresh_files(AppState& state);
void update_file_info(AppState& state, std::vector<std::string>& filenames, 
std::vector<std::string>& datetime, 
std::vector<std::string>& sizes, 
std::vector<std::string>& formats);
void read_file(const fs::path& filepath, std::vector<std::string>& file_lines);
