
#include "structures.h"
#include "ui.h"
#include "file_operations.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include <iostream>

int main() {
    AppState state;
    state.current_path = std::filesystem::current_path();
    refresh_files(state);
    std::vector<std::string> filenames, datetime;
    
    update_file_info(state, filenames, datetime);
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    
    auto setup = setup_ui(screen, state, filenames, datetime);
    screen.Loop(setup);
    return 0;
}