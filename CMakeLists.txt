cmake_minimum_required(VERSION 3.5)

project(docs++)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TEST "Enable the tests" OFF)
option(ENABLE_DEMO "Enable building demo applications" OFF)

include(FetchContent)
FetchContent_Declare(lucene++ 
    GIT_REPOSITORY https://github.com/luceneplusplus/LucenePlusPlus.git
    GIT_TAG df65bf5ff43cee1d79f37217bcbf423f89b07d97)
FetchContent_MakeAvailable(lucene++)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

add_subdirectory(src)
