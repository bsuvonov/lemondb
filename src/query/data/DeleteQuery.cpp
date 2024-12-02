#include "DeleteQuery.h"

#include <memory>
#include <utility>

#include "../../db/Database.h"
#include "../QueryResult.h"

// constexpr const char *DeleteQuery::qname;

QueryResult::Ptr DeleteQuery::execute() {
  Database &db = Database::getInstance();

  try {
    auto &table = db[this->tableName()];
    auto result = this->initCondition(table);

    int deletedRows = 0;
    if (result.second) {
      for (auto it = table.begin(); it != table.end();) {
        if (this->evalCondition(*it)) {
          deletedRows++;
          it = table.erase(it);
        } else {
          it++;
        }
      }
    }
    return std::make_unique<RecordCountResult>(deletedRows);
  } catch (const TableNameNotFound &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "No such table.");
  } catch (const IllFormedQueryCondition &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(), e.what());
  } catch (const std::invalid_argument &e) {
    // Cannot convert operand to string
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "Unknown error '?'"_f % e.what());
  } catch (const std::exception &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "Unkonwn error '?'."_f % e.what());
  }
}

std::string DeleteQuery::toString() {
  return "QUERY = DELETE " + this->tableName() + "\"";
}