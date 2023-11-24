#include "Engine.h"

#include <Constants.h>
#include <FSDirectory.h>
#include <IndexReader.h>
#include <QueryParser.h>
#include <SnowballAnalyzer.h>
#include <StringUtils.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string_view>

using Lucene::newLucene;

Engine::Engine(const std::string &index,
               Lucene::Collection<Lucene::String> fields) {
  auto version = Lucene::LuceneVersion::LUCENE_CURRENT;

  reader = Lucene::IndexReader::open(
      Lucene::FSDirectory::open(Lucene::StringUtils::toUnicode(index)), true);
  searcher = newLucene<Lucene::IndexSearcher>(reader);
  analyzer = newLucene<Lucene::SnowballAnalyzer>(version, L"english");
  analyzer = newLucene<Lucene::StandardAnalyzer>(version);
  parser = newLucene<Lucene::MultiFieldQueryParser>(version, fields, analyzer);
}

Engine::~Engine() { reader->close(); }

void Engine::run() {
  try {
    while (true) {
      std::wstring line;

      std::wcout << "Enter query: ";
      std::wcin >> line;

      boost::trim(line);
      if (line.empty()) {
        break;
      }

      Lucene::QueryPtr query = parser->parse(line);

      search(query, 10);

      std::wstring choice;
      std::wcout << "Do you want to make another query? (y/n): ";
      std::wcin >> choice;

      boost::trim(choice);
      if (choice.starts_with('n')) {
        break;
      }
    }
  } catch (Lucene::LuceneException &e) {
    std::wcout << L"Lucene exception: " << e.getError() << L"\n";
    return;
  }
}

static void displayResultInfo(int idx, Lucene::DocumentPtr doc) {
  Lucene::String title = doc->get(L"title");

  std::wcout << Lucene::StringUtils::toString(idx + 1) + L". " << title << '\n';
  std::wcout << L"Description: " << doc->get(L"signature") << '\n';
}

static std::wstring collectMoreHint(size_t hitsSize, int32_t numTotalHits) {
  std::wcout << "Only results 1 - " << hitsSize << " of " << numTotalHits
             << " total matching documents collected.\n";
  std::wcout << "Collect more (y/n)?";

  std::wstring choice;
  std::wcin >> choice;
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
  std::wcin >> choice;
  boost::trim(choice);

  return choice;
}

void Engine::search(const Lucene::QueryPtr &query, int32_t hitsPerPage) {
  Lucene::TopScoreDocCollectorPtr collector =
      Lucene::TopScoreDocCollector::create(5 * hitsPerPage, false);

  searcher->search(query, collector);

  Lucene::Collection<Lucene::ScoreDocPtr> hits =
      collector->topDocs()->scoreDocs;

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
