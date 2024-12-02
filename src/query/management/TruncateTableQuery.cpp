#include "TruncateTableQuery.h"

#include <memory>
#include <string>

#include "../../db/Database.h"
// constexpr const char *TruncateTableQuery::qname;

QueryResult::Ptr TruncateTableQuery::execute() {
  try {
    auto &db = Database::getInstance();
    auto &tb = db[this->tableName()];
    auto re = tb.clear();
    return std::make_unique<SuccessMsgResult>(qname, this->tableName(),
                                              std::to_string(re));
  } catch (TableNameNotFound &e) {
    return std::make_unique<ErrorMsgResult>(qname, e.what());
  }
}

std::string TruncateTableQuery::toString() {
  return "QUERY = TRUNCATE, Table = \"" + tableName() + "\"";
}
