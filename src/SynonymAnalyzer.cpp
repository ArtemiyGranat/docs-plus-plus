#include "SynonymAnalyzer.h"

#include "SynonymTokenizer.h"

SynonymAnalyzer::SynonymAnalyzer(Lucene::MapStringString synonyms)
    : synonyms(synonyms) {}

SynonymAnalyzer::~SynonymAnalyzer() {}

Lucene::TokenStreamPtr
SynonymAnalyzer::tokenStream(const Lucene::String &fieldName,
                             const Lucene::ReaderPtr &reader) {
  Lucene::LowerCaseTokenizerPtr stream =
      newLucene<Lucene::LowerCaseTokenizer>(reader);
  stream->addAttribute<Lucene::TermAttribute>();
  stream->addAttribute<Lucene::PositionIncrementAttribute>();
  stream->addAttribute<Lucene::OffsetAttribute>();
  return newLucene<SynonymTokenizer>(stream, synonyms);
}
