#include "Engine.h"

#include <Constants.h>
#include <FSDirectory.h>
#include <IndexReader.h>
#include <QueryParser.h>
#include <SnowballAnalyzer.h>
#include <StringUtils.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>

using Lucene::newLucene;

Engine::Engine(const std::string &index,
               Lucene::Collection<Lucene::String> fields) {
  auto version = Lucene::LuceneVersion::LUCENE_CURRENT;

  reader = Lucene::IndexReader::open(
      Lucene::FSDirectory::open(Lucene::StringUtils::toUnicode(index)), true);
  searcher = newLucene<Lucene::IndexSearcher>(reader);
  analyzer = newLucene<Lucene::SnowballAnalyzer>(version, L"english");
  parser = newLucene<Lucene::MultiFieldQueryParser>(version, fields, analyzer);
}

Engine::~Engine() { reader->close(); }

void Engine::processQuery(std::string &query) {
  std::string delimiters[] = {" ", "_", "::", "(", ")", "<", ">"};
  std::string specialSymbols[] = {":",  "(", ")", "[", "]", "{", "}", "&&",
                                  "||", "+", "-", "!", "^", "*", "?"};

  for (const auto &delimiter : delimiters) {
    size_t pos = query.find(delimiter);
    while (pos != std::string::npos) {
      if (pos == 0) {
        pos = query.find(delimiter, pos + 1);
        continue;
      }
      query.replace(pos, delimiter.length(), "~" + delimiter);
      pos = query.find(delimiter, pos + 2);
    }
  }

  for (const auto &symbol : specialSymbols) {
    if (query == symbol || query == "~") {
      std::cout << "Invalid query: " << query << '\n';
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
  query += '~';
}

void Engine::run() {
  try {
    while (true) {
      std::string line;

      std::wcout << "Enter query: ";
      std::getline(std::cin, line);

      Engine::processQuery(line);
      if (line.empty()) {
        continue;
      }

      std::wstring unicodeQuery = Lucene::StringUtils::toUnicode(line);

      Lucene::QueryPtr query = parser->parse(unicodeQuery);

      search(query, 10);

      std::wstring choice;
      std::wcout << "Do you want to make another query? (y/n): ";
      std::getline(std::wcin, choice);

      boost::trim(choice);
      if (choice.starts_with('n')) {
        break;
      }
    }
  } catch (Lucene::LuceneException &e) {
    std::wcerr << "Lucene exception: " << e.getError() << "\n";
    return;
  }
}

static void displayResultInfo(int idx, Lucene::DocumentPtr doc) {
  Lucene::String title = doc->get(L"title");

  std::wcout << Lucene::StringUtils::toString(idx + 1) + L". " << title << '\n';
}

static std::wstring collectMoreHint(size_t hitsSize, int32_t numTotalHits) {
  std::wcout << "Only results 1 - " << hitsSize << " of " << numTotalHits
             << " total matching documents collected.\n";
  std::wcout << "Collect more (y/n)?";

  std::wstring choice;
  std::getline(std::wcin, choice);
  boost::trim(choice);

  return choice;
}

static std::wstring navigateHint(int32_t hitsPerPage, int32_t start,
                                 int32_t numTotalHits) {
  std::wcout << "Press ";
  if (start - hitsPerPage >= 0) {
    std::wcout << "(p)revious page, ";
  }
  if (start + hitsPerPage < numTotalHits) {
    std::wcout << "(n)ext page, ";
  }
  std::wcout << "(q)uit or enter number to jump to a page: ";

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
  std::wcout << numTotalHits << " total matching documents\n";

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
          } catch (Lucene::NumberFormatException &) {
          }
          if ((page - 1) * hitsPerPage < numTotalHits) {
            start = std::max(0, (page - 1) * hitsPerPage);
            break;
          } else {
            std::wcout << "No such page\n";
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
