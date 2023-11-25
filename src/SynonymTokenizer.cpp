#include "SynonymTokenizer.h"

SynonymTokenizer::SynonymTokenizer(const Lucene::TokenStreamPtr &realStream,
                                   Lucene::MapStringString synonyms) {
  this->realStream = realStream;
  this->synonyms = synonyms;
  this->synonymToken = 0;
  this->realTermAtt = realStream->addAttribute<Lucene::TermAttribute>();
  this->realPosIncrAtt =
      realStream->addAttribute<Lucene::PositionIncrementAttribute>();
  this->realOffsetAtt = realStream->addAttribute<Lucene::OffsetAttribute>();

  this->termAtt = addAttribute<Lucene::TermAttribute>();
  this->posIncrAtt = addAttribute<Lucene::PositionIncrementAttribute>();
  this->offsetAtt = addAttribute<Lucene::OffsetAttribute>();
}

SynonymTokenizer::~SynonymTokenizer() {}

bool SynonymTokenizer::incrementToken() {
  if (!currentRealToken) {
    bool next = realStream->incrementToken();
    if (!next) {
      return false;
    }
    clearAttributes();
    termAtt->setTermBuffer(realTermAtt->term());
    offsetAtt->setOffset(realOffsetAtt->startOffset(),
                         realOffsetAtt->endOffset());
    posIncrAtt->setPositionIncrement(realPosIncrAtt->getPositionIncrement());

    if (!synonyms.contains(realTermAtt->term())) {
      return true;
    }
    Lucene::String expansions = synonyms.get(realTermAtt->term());
    synonymTokens = Lucene::StringUtils::split(expansions, L",");
    synonymToken = 0;
    if (!synonymTokens.empty()) {
      currentRealToken = Lucene::newLucene<Lucene::Token>(
          realOffsetAtt->startOffset(), realOffsetAtt->endOffset());
      currentRealToken->setTermBuffer(realTermAtt->term());
    }
    return true;
  } else {
    Lucene::String tok = synonymTokens[synonymToken++];
    clearAttributes();
    termAtt->setTermBuffer(tok);
    offsetAtt->setOffset(currentRealToken->startOffset(),
                         currentRealToken->endOffset());
    posIncrAtt->setPositionIncrement(0);
    if (synonymToken == synonymTokens.size()) {
      currentRealToken.reset();
      synonymTokens.reset();
      synonymToken = 0;
    }
    return true;
  }
}
