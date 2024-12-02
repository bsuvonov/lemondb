//
// Created by liu on 18-10-25.
//

#include "UpdateQuery.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../db/Database.h"

// constexpr const char *UpdateQuery::qname;

QueryResult::Ptr UpdateQuery::execute() {
  if (this->getOperands().size() != 2)
    return make_unique<ErrorMsgResult>(
        qname, this->tableName().c_str(),
        "Invalid number of operands (? operands)."_f % getOperands().size());
  Database &db = Database::getInstance();
  Table::SizeType counter = 0;
  try {
    auto &table = db[this->tableName()];
    if (this->getOperands()[0] == "KEY") {
      this->keyValue = this->getOperands()[1];
    } else {
      this->fieldId = table.getFieldIndex(this->getOperands()[0]);
      this->fieldValue =
          (Table::ValueType)strtol(this->getOperands()[1].c_str(), nullptr, 10);
    }
    auto op = [this](Table::Object object, void *data) {
      if (this->keyValue.empty()) {
        object[this->fieldId] = this->fieldValue;
      } else {
        object.setKey(this->keyValue);
      }
      ++(*(size_t *)data);
    };
    auto data_ptr = new size_t[get_thread_num()];
    std::vector<void *> data{};
    for (size_t i = 0; i < get_thread_num(); ++i) {
      *(data_ptr + i) = 0;
      data.push_back(data_ptr + i);
    }
    auto result = initCondition(table);
    if (result.second) {
      parallel_operate(op, data, &table);

      // for (auto it = table.begin(); it != table.end(); ++it) {
      //   if (this->evalCondition(*it)) {
      //     if (this->keyValue.empty()) {
      //       (*it)[this->fieldId] = this->fieldValue;
      //     } else {
      //       it->setKey(this->keyValue);
      //     }
      //     ++counter;
      //   }
      // }
    }
    for (auto pt : data) {
      counter += *(size_t *)pt;
    }
    delete[] data_ptr;
    return std::make_unique<RecordCountResult>(counter);
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

std::string UpdateQuery::toString() {
  return "QUERY = UPDATE " + this->tableName() + "\"";
}
