#include "ui.h"
#include "file_operations.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <cstdlib>
#include <memory>

using namespace ftxui;

Component setup_ui(ScreenInteractive& screen,
                            AppState& state,
                            std::vector<std::string>& filenames,
                            std::vector<std::string>& datetime,
                            std::vector<std::string>& sizes,
                            std::vector<std::string>& formats){

    auto window_box = std::make_shared<Box>();
    auto opt = MenuOption::Vertical();
    opt.entries_option.transform = [window_box](const EntryState& entry_state) {
        int width = window_box -> x_max - window_box -> x_min;
        if (width <= 0) width = 100;

        Element e = paragraph((entry_state.active ? "> " : "  ") + entry_state.label) | size(WIDTH, LESS_THAN, window_box->x_max - window_box->x_min);
        
        if (entry_state.active) {
            e |= bold | color(Color::Green);
        }
        return e;
    };
    auto file_viewer = Menu(&state.file_lines, &state.file_viewer_dummy, opt);
    auto filenames_menu = Menu(&filenames, &state.selected_index, opt);
    auto datetimes_menu = Menu(&datetime, &state.selected_index, opt);
    auto sizes_menu = Menu(&sizes, &state.selected_index, opt);
    auto formats_menu = Menu(&formats, &state.selected_index, opt);
    auto container = Container::Horizontal({
        filenames_menu,
        datetimes_menu,
        sizes_menu,
        formats_menu
    });
    
    auto component = CatchEvent(container, [&screen, &state, &filenames, &datetime, &sizes, &formats, file_viewer](Event e) {
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
                update_file_info(state, filenames, datetime, sizes, formats);
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
                        update_file_info(state, filenames, datetime, sizes, formats);
                    }
                } else {
                    fs::path next = state.current_path / name;
                    if (fs::exists(next) && fs::is_directory(next)) {
                        state.current_path = next;
                        state.text_display = 0;
                        refresh_files(state);
                        update_file_info(state, filenames, datetime, sizes, formats);
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
                    update_file_info(state, filenames, datetime, sizes, formats);
                }
            }
            return true;
        }
        return false;

    });

    auto renderer = Renderer(component, [&state, file_viewer, filenames_menu, datetimes_menu, sizes_menu, formats_menu, window_box] {
        auto header = hbox({
            text(" FTXUI File Manager ") | bold | bgcolor(Color::Blue) | color(Color::White),
            text(" ") | flex,
            text(state.current_path.string()) | color(Color::Yellow),
        });

        Element file_list;
        if (state.text_display) {
            file_list = vbox(file_viewer->Render() | frame | flex) | flex;
        } else {
           
            auto col_name   = vbox({ text(" Имя ")    | bold | dim, filenames_menu->Render() | flex }) | flex;
            auto col_date   = vbox({ text(" Изменён ")| bold | dim, datetimes_menu->Render() | flex });
            auto col_size   = vbox({ text(" Размер ") | bold | dim, sizes_menu->Render()     | flex });
            auto col_format = vbox({ text(" Формат ") | bold | dim, formats_menu->Render()   | flex });

            file_list = hbox({ col_name, col_date, col_size, col_format }) | flex;
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

        return vbox({header, file_list,  footer}) | flex | reflect(*window_box);
    });

    return renderer;
}
