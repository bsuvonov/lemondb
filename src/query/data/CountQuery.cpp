#include "CountQuery.h"

#include <memory>
#include <string>
#include <vector>

#include "../../db/Database.h"

QueryResult::Ptr CountQuery::execute() {
  Database &db = Database::getInstance();
  auto &table = db[this->tableName()];

  if (!this->getOperands().empty())
    return make_unique<ErrorMsgResult>(
        qname, this->tableName().c_str(),
        "Invalid number of operands (? operands)."_f % getOperands().size());

  // Initialize counts for the field

  try {
    // vector<int> counts;
    size_t counter = 0;
    auto data_ptr = new size_t[get_thread_num()];
    std::vector<void *> data{};
    for (size_t i = 0; i < get_thread_num(); ++i) {
      *(data_ptr + i) = 0;
      data.push_back(data_ptr + i);
    }
    auto op = [](Table::Object /*object*/, void *data) { ++(*(size_t *)data); };
    auto result = initCondition(table);
    if (result.second) {
      parallel_operate(op, data, &table);
      // for (auto it = table.begin(); it != table.end(); ++it) {
      //   if (this->evalCondition(*it)) {
      //     counts.push_back((*it)[0]);
      //   }
      // }
      // counter = static_cast<int>(counts.size());
    }
    for (auto pt : data) {
      counter += *(size_t *)pt;
    }
    delete[] data_ptr;
    return std::make_unique<SuccessMsgResult>(counter);

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
                                            "Unknown error '?'."_f % e.what());
  }
}

// TODO: implement `combine` method for multi-threading
// for milestone 2

std::string CountQuery::toString() {
  return "QUERY = COUNT " + this->tableName() + "\"";
}
