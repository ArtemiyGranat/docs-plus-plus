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
  std::unordered_map<std::string, std::vector<std::string>> synonymsMap;

  void addSynonyms(std::string_view fileName);
public:
  explicit Engine(const std::string &index,
                  Lucene::Collection<Lucene::String> fields);
  ~Engine();
  void run();
  void search(const Lucene::QueryPtr &query, int32_t hitsPerPage);
  void processQuery(std::string &query);

public:
  Engine(const Engine &) = delete;
  Engine(Engine &&) = delete;
  Engine &operator=(const Engine &) = delete;
  Engine &operator=(Engine &&) = delete;
};

#endif // ENGINE_H
