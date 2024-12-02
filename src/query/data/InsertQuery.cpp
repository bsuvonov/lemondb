//
// Created by liu on 18-10-25.
//

#include "InsertQuery.h"

#include <memory>
#include <utility>
#include <vector>

#include "../../db/Database.h"
#include "../QueryResult.h"

// constexpr const char *InsertQuery::qname;

QueryResult::Ptr InsertQuery::execute() {
  if (this->getOperands().empty())
    return make_unique<ErrorMsgResult>(qname, this->tableName().c_str(),
                                       "No operand (? operands)."_f %
                                           getOperands().size());
  Database &db = Database::getInstance();

  try {
    auto &table = db[this->tableName()];
    auto &key = this->getOperands().front();
    std::vector<Table::ValueType> data;
    data.reserve(this->getOperands().size() - 1);
    for (auto it = ++this->getOperands().begin();
         it != this->getOperands().end(); ++it) {
      data.emplace_back(strtol(it->c_str(), nullptr, 10));
    }
    table.insertByIndex(key, std::move(data));
    return std::make_unique<SuccessMsgResult>(qname, tableName(), false);
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

std::string InsertQuery::toString() {
  return "QUERY = INSERT " + this->tableName() + "\"";
}
