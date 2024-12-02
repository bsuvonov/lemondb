
#include "SumQuery.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../db/Database.h"

QueryResult::Ptr SumQuery::execute() {
  Database &db = Database::getInstance();
  auto &table = db[this->tableName()];

  // Initialize sums for each field
  std::vector<int> sums(this->getOperands().size(), 0);

  try {
    auto result = initCondition(table);
    if (result.second) {
      processTableRows(&table, &sums);
    }

  } catch (const TableNameNotFound &e) {
    return make_unique<ErrorMsgResult>(qname, this->tableName(),
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
  auto res = make_unique<SuccessMsgResult>(std::move(sums));
  return res;
}

// TODO: implement `combine` method for multi-threading
// for milestone 2

std::string SumQuery::toString() {
  return "QUERY = SUM " + this->tableName() + "\"";
}

void SumQuery::processTableRows(Table *table, std::vector<int> *sums) {
  std::vector<Table::FieldIndex> field_id_list{};
  field_id_list.reserve(this->getOperands().size());
  for (const auto &i : this->getOperands())
    field_id_list.emplace_back((*table).getFieldIndex(i));

  auto ans_size = this->getOperands().size();
  auto data_ptr = new std::vector<int>[get_thread_num()];
  std::vector<void *> data{};
  for (size_t i = 0; i < get_thread_num(); ++i) {
    (data_ptr + i)->resize(ans_size);
    // (data_ptr+i).
    data.push_back(data_ptr + i);
  }
  auto op = [&field_id_list](Table::Object object, void *data) {
    for (size_t i = 0; i < field_id_list.size(); ++i) {
      (*(std::vector<int> *)data)[i] += object[field_id_list[i]];
    }
  };
  parallel_operate(op, data, table);
  for (auto th_s : data) {
    for (size_t i = 0; i < field_id_list.size(); ++i)
      (*sums)[i] += (*(std::vector<int> *)th_s)[i];
  }
  delete[] data_ptr;

  // for (const auto &row : table) {
  //   if (this->evalCondition(row)) {
  //     for (size_t i = 0; i < this->getOperands().size(); ++i) {
  //       sums[i] += row[fieldIndex];
  //     }
  //   }
  // }
}
