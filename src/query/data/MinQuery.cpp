
#include "MinQuery.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../../db/Database.h"

std::vector<int>
MinQuery::find_min(const std::vector<Table::FieldIndex> &field_id_list,
                   const Table::Iterator &start, const Table::Iterator &end) {
  std::vector<int> mins(field_id_list.size(), std::numeric_limits<int>::max());
  for (auto it = start; it != end; ++it) {
    if (this->evalCondition(*it)) {
      find = true;
      for (size_t i = 0; i < field_id_list.size(); ++i) {
        mins[i] = std::min(mins[i], (*it)[field_id_list[i]]);
      }
    }
  }

  return mins;
}

QueryResult::Ptr MinQuery::execute() {
  Database &db = Database::getInstance();
  try {
    auto &table = db[this->tableName()];

    std::vector<int> mins(this->getOperands().size(),
                          std::numeric_limits<int>::max());

    std::vector<Table::FieldIndex> filed_id_list(this->getOperands().size());

    for (std::size_t j = 0; j < this->getOperands().size(); ++j) {
      filed_id_list[j] = (table.getFieldIndex(this->getOperands()[j]));
    }

    auto op = [&filed_id_list](Table::Object object, void *data) {
      for (size_t j = 0; j < filed_id_list.size(); ++j) {
        (*(std::vector<int> *)data)[j] = std::min<Table::ValueType>(
            (*(std::vector<int> *)data)[j], object[filed_id_list[j]]);
      }
    };
    auto data_ptr = new std::vector<int>[get_thread_num()];
    std::vector<void *> data{};
    for (size_t i = 0; i < get_thread_num(); ++i) {
      *(data_ptr + i) = mins;
      data.push_back(data_ptr + i);
    }
    auto result = initCondition(table);
    if (result.second)
      parallel_operate(op, data, &table);

    for (auto pt : data) {
      for (size_t j = 0; j < filed_id_list.size(); ++j) {
        mins[j] =
            std::min<Table::ValueType>(mins[j], (*(std::vector<int> *)pt)[j]);
      }
    }
    delete[] data_ptr;
    find = (mins[0] != std::numeric_limits<int>::max());

    if (find)
      return std::make_unique<SuccessMsgResult>(std::move(mins));
    else
      return std::make_unique<SuccessMsgResult>(qname);
  } catch (const TableNameNotFound &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "No such table.");
  } catch (const IllFormedQueryCondition &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(), e.what());
  } catch (const std::invalid_argument &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "Unknown error '?'"_f % e.what());
  } catch (const std::exception &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "Unknown error '?'."_f % e.what());
  }
}

std::string MinQuery::toString() {
  return "QUERY = MIN " + this->tableName() + "\"";
}
