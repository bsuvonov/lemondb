//
// Created by liu on 18-10-25.
//

#include "DumpTableQuery.h"

#include <fstream>
#include <memory>
#include <string>

#include "../../db/Database.h"

// constexpr const char *DumpTableQuery::qname;

QueryResult::Ptr DumpTableQuery::execute() {
  auto &db = Database::getInstance();
  try {
    std::ofstream outfile(this->fileName);
    if (!outfile.is_open()) {
      return make_unique<ErrorMsgResult>(qname, "Cannot open file '?'"_f %
                                                    this->fileName);
    }
    outfile << db[this->tableName()];
    outfile.close();
    return make_unique<SuccessMsgResult>(qname, tableName());
  } catch (const std::exception &e) {
    return std::make_unique<ErrorMsgResult>(qname, e.what());
  }
}

std::string DumpTableQuery::toString() {
  return "QUERY = Dump TABLE, FILE = \"" + this->fileName + "\"";
}
