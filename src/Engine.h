#ifndef ENGINE_H
#define ENGINE_H

#include <LuceneHeaders.h>
#include <LuceneTypes.h>

class Engine {
private:
  Lucene::IndexReaderPtr reader;
  Lucene::SearcherPtr searcher;
  Lucene::AnalyzerPtr analyzer;
  Lucene::MultiFieldQueryParserPtr parser;

public:
  explicit Engine(const std::string &index,
                  Lucene::Collection<Lucene::String> fields);
  ~Engine();
  void run();
  void search(const Lucene::QueryPtr &query, int32_t hitsPerPage);

public:
  Engine(const Engine &) = delete;
  Engine(Engine &&) = delete;
  Engine &operator=(const Engine &) = delete;
  Engine &operator=(Engine &&) = delete;
};

#endif // ENGINE_H
