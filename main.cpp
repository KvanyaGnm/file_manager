#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include "ftxui/dom/table.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <format>
#include <algorithm>
#include <cstdlib>
#include <fstream>

namespace fs = std::filesystem;
using namespace ftxui;

struct Object {
    std::string name;
    std::filesystem::file_time_type modification_time;
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
        filenames.push_back(x.name);

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

int main() {
    AppState state;
    state.current_path = fs::current_path();
    refresh_files(state);
    std::vector<std::string> filenames, datetime;
    
    update_file_info(state, filenames, datetime);
    auto screen = ScreenInteractive::Fullscreen();
    Box window_box;
    auto opt = MenuOption::Vertical();
    opt.entries_option.transform = [&](const EntryState& state) {

        Element e = paragraph((state.active ? "> " : "  ") + state.label) | size(WIDTH, LESS_THAN, window_box.x_max - window_box.x_min);
        
        if (state.active) {
            e |= bold | color(Color::Green);
        }
        return e;
    };
    auto file_viewer = Menu(&state.file_lines, &state.file_viewer_dummy, opt);
    auto filenames_menu = Menu(&filenames, &state.selected_index, opt);
    auto datetimes_menu = Menu(&datetime, &state.selected_index, opt);
    
    auto container = Container::Horizontal({
        filenames_menu,
        datetimes_menu
    });
    
    auto component = CatchEvent(container, [&](Event e) {
        if (e == Event::Escape) {
            screen.Exit();
            return true;
        }

        if (state.text_display) {
            if (e == Event::Backspace) {
                state.text_display = 0;
                return true;
            }
            return file_viewer->OnEvent(e);
        }

        
        if (e == Event::Backspace) {
            if (state.current_path != state.current_path.root_path()) {
                state.current_path = state.current_path.parent_path();
                refresh_files(state);
                update_file_info(state, filenames, datetime);
            }
            
            return true;
        }
        if (e == Event::Return) {
            if (state.selected_index >= 0 && state.selected_index < static_cast<int>(filenames.size())) {
                auto name = filenames[state.selected_index];
                if (name == "..") {
                    if (state.current_path != state.current_path.root_path()) {
                        state.current_path = state.current_path.parent_path();
                        state.text_display = 0;
                        refresh_files(state);
                        update_file_info(state, filenames, datetime);
                    }
                } else {
                    fs::path next = state.current_path / name;
                    if (fs::exists(next) && fs::is_directory(next)) {
                        state.current_path = next;
                        state.text_display = 0;
                        refresh_files(state);
                        update_file_info(state, filenames, datetime);
                    } else {
                        state.text_display = 1;
                        state.file_viewer_dummy = 0;
                        read_file(next, state.file_lines);
                    }
                }
            }
            return true;
        }
        if (e == Event::Character('e') || e == Event::Character('E')) {
            if (state.selected_index >= 0 && state.selected_index < static_cast<int>(filenames.size())) {
                auto name = filenames[state.selected_index];
                fs::path next = state.current_path / name;
                if (fs::exists(next) && !fs::is_directory(next) && name != "..") {
                    screen.WithRestoredIO([&] {
                        std::system(("nano \"" + next.string() + "\"").c_str());
                    })();                      
                    refresh_files(state);
                    update_file_info(state, filenames, datetime);
                }
            }
            return true;
        }
        return false;

    });

    auto renderer = Renderer(component, [&] {
        auto header = hbox({
            text(" FTXUI File Manager ") | bold | bgcolor(Color::Blue) | color(Color::White),
            text(" ") | flex,
            text(state.current_path.string()) | color(Color::Yellow),
        });

        Element file_list;
        if (state.text_display){
            file_list = vbox(file_viewer->Render()| frame | flex) | flex ;
        }else{ 
            file_list= hbox({vbox({
                text(" Имя ") | bold | dim,
                filenames_menu->Render() | flex
            }) | flex, 
            vbox({
                text(" Изменён ") | bold | dim,
                datetimes_menu->Render() | flex,
            })}) | flex;
        }
        auto status = state.error.empty() 
            ? text("OK") | color(Color::Green)
            : text(state.error) | color(Color::Red);

        auto footer = vbox({
            separator(),
            hbox({
                text(" <->: Навигация | Enter: Войти | Backspace: Наверх | Esc: Выход "),
                text(" ") | flex,
                status,
            }) | bgcolor(Color::GrayDark) | color(Color::White),
        });

        return vbox({header, file_list,  footer}) | flex | reflect(window_box);
    });

    screen.Loop(renderer);
    return 0;
}