#include <LuceneHeaders.h>

class SynonymAnalyzer : public Lucene::Analyzer {
public:
  explicit SynonymAnalyzer(Lucene::MapStringString synonyms);

  virtual ~SynonymAnalyzer();

protected:
  Lucene::MapStringString synonyms;

public:
  virtual Lucene::TokenStreamPtr tokenStream(const Lucene::String &fieldName,
                                             const Lucene::ReaderPtr &reader);
};
