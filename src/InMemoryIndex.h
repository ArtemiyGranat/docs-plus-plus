#ifndef IN_MEMORY_INDEX_H
#define IN_MEMORY_INDEX_H

#include <LuceneHeaders.h>

class InMemoryIndex {
private:
  Lucene::IndexWriterPtr writer;
  void addSynonyms(Lucene::MapStringString synonymsMap,
                   const std::string &fileName);

public:
  explicit InMemoryIndex(const std::string &indexDir);
  ~InMemoryIndex();

  void indexDocument();
  void processJsonFile(const std::string &filePath);

public:
  InMemoryIndex(const InMemoryIndex &) = delete;
  InMemoryIndex(InMemoryIndex &&) = delete;
  InMemoryIndex &operator=(const InMemoryIndex &) = delete;
  InMemoryIndex &operator=(InMemoryIndex &&) = delete;
};

#endif // IN_MEMORY_INDEX_H
