#include "Engine.h"
#include "InMemoryIndex.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <fmt/color.h>
#include <fmt/core.h>

void indexFiles(char *argv[]) {
  std::string sourceFile(argv[1]);
  std::string indexDir(argv[2]);

  if (!std::filesystem::exists(sourceFile)) {
    std::stringstream msg;
    msg << "Source file doesn't exist: " << sourceFile;
    throw std::runtime_error(msg.str());
  }

  if (!std::filesystem::is_directory(indexDir)) {
    if (!std::filesystem::create_directory(indexDir)) {
      std::stringstream msg;
      msg << "Unable to create directory: " << indexDir;
      throw std::runtime_error(msg.str());
    }
  }

  InMemoryIndex index(indexDir);
  index.processJsonFile(sourceFile);
}

int main(int argc, char *argv[]) {
  // TODO: argv[0] --index <source> <index> or argv[0] <index> or usage message
  if (argc == 3) {
    try {
      indexFiles(argv);
    } catch (const std::runtime_error &e) {
      fmt::println("{} {}",
                   fmt::styled("✗", fmt::fg(fmt::terminal_color::red)), e.what());
    }
    return EXIT_FAILURE;
  }

  std::string index = argv[1];
  if (!std::filesystem::is_directory(index)) {
     fmt::println("{} Index directory doesn't exist: {}",
                   fmt::styled("✗", fmt::fg(fmt::terminal_color::red)), index);
    return EXIT_FAILURE;
  }
  // TODO: read fields from file?
  Lucene::Collection<Lucene::String> fields =
      Lucene::newCollection<Lucene::String>(L"title", L"headers",
                                            L"description");

  Engine engine(index, fields);
  engine.run();
}
