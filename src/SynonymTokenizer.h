#include <LuceneHeaders.h>
#include <OffsetAttribute.h>
#include <PositionIncrementAttribute.h>
#include <TermAttribute.h>

class SynonymTokenizer : public Lucene::TokenStream {
public:
  explicit SynonymTokenizer(const Lucene::TokenStreamPtr &realStream,
                            Lucene::MapStringString synonyms);
  virtual ~SynonymTokenizer();

protected:
  Lucene::TokenStreamPtr realStream;
  Lucene::TokenPtr currentRealToken;
  Lucene::TokenPtr cRealToken;
  Lucene::MapStringString synonyms;
  Lucene::Collection<Lucene::String> synonymTokens;
  int32_t synonymToken;
  Lucene::TermAttributePtr realTermAtt;
  Lucene::PositionIncrementAttributePtr realPosIncrAtt;
  Lucene::OffsetAttributePtr realOffsetAtt;
  Lucene::TermAttributePtr termAtt;
  Lucene::PositionIncrementAttributePtr posIncrAtt;
  Lucene::OffsetAttributePtr offsetAtt;

public:
  virtual bool incrementToken();
};
