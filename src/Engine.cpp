#include <iostream>
#include <regex>
#include <string>
#include <string_view>

#include <Constants.h>
#include <FSDirectory.h>
#include <IndexReader.h>
#include <QueryParser.h>
#include <SnowballAnalyzer.h>
#include <StringUtils.h>

#include <boost/algorithm/string.hpp>

#include <fmt/color.h>
#include <fmt/ranges.h>
#include <fmt/xchar.h>

#include <nlohmann/json.hpp>

#include "Engine.h"

using Lucene::newLucene;

Engine::Engine(const std::string &index,
               Lucene::Collection<Lucene::String> fields) {
  auto version = Lucene::LuceneVersion::LUCENE_CURRENT;

  reader = Lucene::IndexReader::open(
      Lucene::FSDirectory::open(Lucene::StringUtils::toUnicode(index)), true);
  searcher = newLucene<Lucene::IndexSearcher>(reader);
  analyzer = newLucene<Lucene::SnowballAnalyzer>(version, L"english");
  parser = newLucene<Lucene::MultiFieldQueryParser>(version, fields, analyzer);
  addSynonyms("synonyms.json");
}

void Engine::addSynonyms(std::string_view fileName) {
  std::ifstream file(fileName);
  if (file.is_open()) {
    nlohmann::json data;
    file >> data;

    for (auto &[term, synonyms] : data.items()) {
      synonymsMap[term].insert(synonymsMap[term].end(), synonyms.begin(),
                               synonyms.end());
    }

    file.close();
  } else {
    std::cerr << "Unable to open file: " << fileName << std::endl;
  }
}

Engine::~Engine() { reader->close(); }

void Engine::processQuery(std::string &query) {
  std::string delimiters[] = {" ", "_", "::", "(", ")", "<", ">"};
  std::string specialSymbols[] = {":",  "(", ")", "[", "]", "{", "}",
                                  "||", "+", "-", "!", "*", "?"};

  for (const auto &symbol : specialSymbols) {
    if (query == symbol || query == "~") {
      fmt::println("✖ Invalid query: {}", query);
      query = "";
      return;
    }
    std::string pattern = "";
    for (const auto &ch : symbol) {
      pattern += std::string("\\") + ch;
    }
    query =
        std::regex_replace(query, std::regex(std::string(pattern)), pattern);
  }
  boost::trim(query);
}

void Engine::run() {
  try {
    while (true) {
      std::string line;

      fmt::print(fmt::fg(fmt::terminal_color::blue), " Enter query ❯ ");
      std::getline(std::cin, line);

      Engine::processQuery(line);
      if (line.empty()) {
        continue;
      }

      std::istringstream lineStream(line);
      std::istream_iterator<std::string> begin(lineStream), end;
      while (begin != end) {
        for (const auto &synonym : synonymsMap[*begin]) {
          line += " " + synonym;
        }
        ++begin;
      }
      
      std::wstring unicodeQuery = Lucene::StringUtils::toUnicode(line);

      Lucene::QueryPtr query = parser->parse(unicodeQuery);

      search(query, 10);

      std::wstring choice;
      fmt::print("Do you want to make another query? {} ❯ ",
                 fmt::styled("(y/n)", fmt::fg(fmt::terminal_color::blue)));
      std::getline(std::wcin, choice);

      boost::trim(choice);
      if (choice.starts_with('n')) {
        break;
      }
    }
  } catch (Lucene::LuceneException &e) {
    fmt::println(stderr, L"{} Lucene exception: {}",
                 fmt::styled(L"✗", fmt::fg(fmt::terminal_color::red)),
                 e.getError());
    return;
  }
}

static inline void displayResultInfo(int idx, Lucene::DocumentPtr doc) {
  fmt::println(L"{}. {}", idx + 1, doc->get(L"title"));
}

static std::wstring collectMoreHint(size_t hitsSize, int32_t numTotalHits) {
  fmt::println("Only first {} matching docs collected.", hitsSize);
  fmt::print("Collect more? {} ❯ ",
             fmt::styled("(y/n)", fmt::fg(fmt::terminal_color::blue)));

  std::wstring choice;
  std::getline(std::wcin, choice);
  boost::trim(choice);

  return choice;
}

static std::wstring navigateHint(int32_t hitsPerPage, int32_t start,
                                 int32_t numTotalHits) {
  fmt::print(fmt::fg(fmt::terminal_color::blue), "Press ");
  if (start - hitsPerPage >= 0) {
    fmt::print(fmt::fg(fmt::terminal_color::blue), "(p)revious page, ");
  }
  if (start + hitsPerPage < numTotalHits) {
    fmt::print(fmt::fg(fmt::terminal_color::blue), "(n)ext page, ");
  }
  fmt::print(fmt::fg(fmt::terminal_color::blue),
             "(q)uit or enter number to jump to a page ❯ ");

  std::wstring choice;
  std::getline(std::wcin, choice);
  boost::trim(choice);

  return choice;
}

void Engine::search(const Lucene::QueryPtr &query, int32_t hitsPerPage) {
  auto collector = Lucene::TopScoreDocCollector::create(5 * hitsPerPage, false);

  searcher->search(query, collector);

  auto hits = collector->topDocs()->scoreDocs;

  int32_t numTotalHits = collector->getTotalHits();
  if (numTotalHits > 0) {
    fmt::print(fg(fmt::terminal_color::green), "✓ Found {} matches\n",
               numTotalHits);
  } else {
    fmt::print(fmt::fg(fmt::terminal_color::yellow), "Hasn't found anything\n");
  }

  int32_t start = 0;
  int32_t end = std::min(numTotalHits, hitsPerPage);

  while (true) {
    if (end > hits.size()) {
      if (collectMoreHint(hits.size(), numTotalHits).starts_with('n')) {
        break;
      }

      collector = Lucene::TopScoreDocCollector::create(numTotalHits, false);
      searcher->search(query, collector);
      hits = collector->topDocs()->scoreDocs;
    }

    end = std::min(hits.size(), start + hitsPerPage);

    for (int32_t i = start; i < end; ++i) {
      Lucene::DocumentPtr doc = searcher->doc(hits[i]->doc);
      displayResultInfo(i, doc);
    }

    if (numTotalHits <= hitsPerPage) {
      break;
    }

    if (numTotalHits >= end) {
      bool quit = false;
      while (true) {
        std::wstring choice = navigateHint(hitsPerPage, start, numTotalHits);

        if (choice.starts_with('q')) {
          quit = true;
          break;
        }
        if (choice.starts_with('p')) {
          start = std::max(0, start - hitsPerPage);
          break;
        } else if (choice.starts_with('n')) {
          if (start + hitsPerPage < numTotalHits) {
            start += hitsPerPage;
          }
          break;
        } else {
          int32_t page = 0;
          try {
            page = Lucene::StringUtils::toInt(choice);
          } catch (Lucene::NumberFormatException &) { // TODO: catch it properly
            quit = true;
            break;
          }
          if ((page - 1) * hitsPerPage < numTotalHits) {
            start = std::max(0, (page - 1) * hitsPerPage);
            break;
          } else {
            fmt::println("{} No such page",
                         fmt::styled("✖", fmt::fg(fmt::terminal_color::red)));
          }
        }
      }
      if (quit) {
        break;
      }
      end = std::min(numTotalHits, start + hitsPerPage);
    }
  }
}
