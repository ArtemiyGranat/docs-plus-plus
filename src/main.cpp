#include <LuceneHeaders.h>
#include <MiscUtils.h>
#include <StringUtils.h>

#include <filesystem>
#include <iostream>

#include <nlohmann/json.hpp>

using namespace Lucene;
using json = nlohmann::json;

DocumentPtr toDocument(const json &elem) {
  DocumentPtr doc = newLucene<Document>();

  doc->add(newLucene<Field>(L"title", StringUtils::toUnicode(elem["title"]), Field::STORE_YES,
                            Field::INDEX_ANALYZED));
  doc->add(newLucene<Field>(L"headers", StringUtils::toUnicode(elem["headers"]), Field::STORE_YES,
                            Field::INDEX_NOT_ANALYZED));
  doc->add(newLucene<Field>(L"signature", StringUtils::toUnicode(elem["signature"]), Field::STORE_YES,
                            Field::INDEX_ANALYZED));
  doc->add(newLucene<Field>(L"description", StringUtils::toUnicode(elem["description"]),
                            Field::STORE_YES, Field::INDEX_ANALYZED));
  doc->add(newLucene<Field>(L"example", StringUtils::toUnicode(elem["example"]),
                            Field::STORE_YES, Field::INDEX_ANALYZED));

  return doc;
}

void index(const IndexWriterPtr &writer, const std::string &sourceFile) {
  std::ifstream src(sourceFile);
  json data = json::parse(src);

  for (const auto &doc : data) {
    std::cout << "Indexing " << doc["title"] << '\n';
    writer->addDocument(toDocument(doc));
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <source file> <index dir>\n";
    return 1;
  }

  std::string sourceFile(argv[1]);
  std::string indexDir(argv[2]);

  if (!std::filesystem::exists(sourceFile)) {
    std::cout << "Source file doesn't exist: " << sourceFile << "\n";
  }

  if (!std::filesystem::is_directory(indexDir)) {
    if (!std::filesystem::create_directory(indexDir)) {
      std::cout << "Unable to create directory: " << indexDir << "\n";
      return 1;
    }
  }

  try {
    IndexWriterPtr writer = newLucene<IndexWriter>(
        FSDirectory::open(StringUtils::toUnicode(indexDir)),
        newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true,
        IndexWriter::MaxFieldLengthLIMITED);
    std::cout << "Indexing to directory: " << indexDir << '\n';

    index(writer, sourceFile);

    writer->optimize();
    writer->close();
  } catch (LuceneException &e) {
    std::cout << "Exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
