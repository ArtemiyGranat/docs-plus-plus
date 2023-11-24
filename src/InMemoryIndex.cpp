#include "InMemoryIndex.h"

#include <iostream>

#include <LuceneHeaders.h>
#include <MiscUtils.h>
#include <SnowballAnalyzer.h>
#include <StringUtils.h>

#include <nlohmann/json.hpp>

using Lucene::Field;
using Lucene::newLucene;
using Lucene::StringUtils;

InMemoryIndex::InMemoryIndex(const std::string &indexDir) {
  writer = newLucene<Lucene::IndexWriter>(
      Lucene::FSDirectory::open(Lucene::StringUtils::toUnicode(indexDir)),
      newLucene<Lucene::SnowballAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT,
                                          L"english"),
      true, Lucene::IndexWriter::MaxFieldLengthLIMITED);
}

InMemoryIndex::~InMemoryIndex() {}

void InMemoryIndex::processJsonFile(const std::string &filePath) {
  std::ifstream src(filePath);
  nlohmann::json data = nlohmann::json::parse(src);

  for (const auto &elem : data) {
    std::cout << "Indexing " << elem["title"] << '\n';
    Lucene::DocumentPtr doc = newLucene<Lucene::Document>();

    doc->add(newLucene<Field>(L"title", StringUtils::toUnicode(elem["title"]),
                              Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"headers",
                              StringUtils::toUnicode(elem["headers"]),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"signature",
                              StringUtils::toUnicode(elem["signature"]),
                              Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"description",
                              StringUtils::toUnicode(elem["description"]),
                              Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"example",
                              StringUtils::toUnicode(elem["example"]),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    writer->addDocument(doc);
  }

  writer->optimize();
  writer->close();
}
