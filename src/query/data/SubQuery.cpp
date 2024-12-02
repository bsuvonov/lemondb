#include "SubQuery.h"

#include <memory>
#include <utility>

#include "../../db/Database.h"

QueryResult::Ptr SubQuery::execute() {
  if (this->getOperands().size() < 2)
    return make_unique<ErrorMsgResult>(qname, this->tableName().c_str(),
                                       "No enough operand (? operands)."_f %
                                           getOperands().size());
  Database &db = Database::getInstance();

  std::vector<Table::FieldIndex> field_id_list(this->getOperands().size() - 2);
  size_t count = 0;
  try {
    auto &table = db[this->tableName()];
    auto result = initCondition(table);

    for (std::size_t j = 1; j < this->getOperands().size() - 1; ++j) {
      field_id_list[j - 1] = (table.getFieldIndex(this->getOperands()[j]));
    }
    Table::FieldIndex const destid = table.getFieldIndex(
        this->getOperands()[this->getOperands().size() - 1]);
    Table::FieldIndex const startid =
        table.getFieldIndex(this->getOperands()[0]);
    auto op = [&field_id_list, &destid, &startid](Table::Object object,
                                                  void *data) {
      Table::ValueType res = object[startid];
      for (auto &i : field_id_list) {
        res -= object[i];
      }
      object[destid] = res;
      ++(*(size_t *)data);
    };
    auto data_ptr = new size_t[get_thread_num()];
    std::vector<void *> data{};
    for (size_t i = 0; i < get_thread_num(); ++i) {
      *(data_ptr + i) = 0;
      data.push_back(data_ptr + i);
    }
    if (result.second) {
      parallel_operate(op, data, &table);

      // for (auto it = table.begin(); it != table.end(); ++it) {
      //   if (this->evalCondition(*it)) {
      //     Table::ValueType res = (*it)[startid];
      //     for (auto &i : field_id_list) {
      //       res -= (*it)[i];
      //     }
      //     (*it)[destid] = res;
      //     count++;
      //   }
      // }
    }
    for (auto pt : data) {
      count += *(size_t *)pt;
    }
    delete[] data_ptr;
    // std::cout << " Affected ? rows.\n"_f % count;
    return std::make_unique<RecordCountResult>(count);
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
std::string SubQuery::toString() {
  return "QUERY = SUB \"" + this->tableName() + "\"";
}
