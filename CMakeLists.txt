cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(FileManager)

include(FetchContent)
 
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9  # Replace with a version, tag, or commit hash
)
 
FetchContent_MakeAvailable(ftxui)
 
add_executable(FileManager main.cpp)
target_link_libraries(FileManager
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
