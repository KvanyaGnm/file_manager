#pragma once
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include "structures.h"
#include <vector>
#include <string>

ftxui::Component setup_ui(ftxui::ScreenInteractive& screen,
                            AppState& state,
                            std::vector<std::string>& filenames,
                            std::vector<std::string>& datetime);