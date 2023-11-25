set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    main.cpp
    InMemoryIndex.cpp
    Engine.cpp
    SynonymAnalyzer.cpp
    SynonymTokenizer.cpp
)

add_executable(docs++ ${SOURCES})

target_link_libraries(docs++ 
    lucene++::lucene++
    lucene++-contrib
    nlohmann_json::nlohmann_json
)

