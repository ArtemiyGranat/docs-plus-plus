set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    main.cpp
)

add_executable(docs++ ${SOURCES})

target_link_libraries(docs++ 
    lucene++::lucene++
    nlohmann_json::nlohmann_json
)

