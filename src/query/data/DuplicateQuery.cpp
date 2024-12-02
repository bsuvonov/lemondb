#include "DuplicateQuery.h"

#include <memory>
#include <utility>
#include <vector>

#include "../../db/Database.h"

QueryResult::Ptr DuplicateQuery::execute() {
  if (!this->getOperands().empty())
    return make_unique<ErrorMsgResult>(
        qname, this->tableName().c_str(),
        "Should provide 0 operand (provided ? operands)."_f %
            getOperands().size());
  Database &db = Database::getInstance();
  Table::SizeType counter = 0;

  try {
    auto &table = db[this->tableName()];
    auto row_size = table.field().size();
    ;

    auto op = [&row_size](Table::Object object, void *data) {
      row_type new_row(row_size);
      for (size_t i = 0; i < row_size; ++i)
        new_row[i] = object[i];
      (*(record_type *)data).emplace_back(object.key(), std::move(new_row));
    };

    auto data_ptr = new record_type[get_thread_num()];
    std::vector<void *> data{};
    for (size_t i = 0; i < get_thread_num(); ++i) {
      data.push_back(data_ptr + i);
    }

    auto result = initCondition(table);
    if (result.second) {
      parallel_operate(op, data, &table);
      // for (auto it : table) {
      //   if (this->evalCondition(it)) {
      //     vector<Table::ValueType> values(table.field().size());
      //     for (std::size_t i = 0; i < table.field().size(); ++i) {
      //       values[i] = it[i];
      //     }
      //     pair<string, vector<Table::ValueType>> const p = {it.key(),
      //                                                       std::move(values)};
      //     new_records.emplace_back(p);
      //   }
      // }
      // try to add records
      for (auto new_records : data)
        for (auto it : (*(record_type *)new_records)) {
          try {
            auto new_name = it.first + "_copy";
            table.insertByIndex(new_name, std::move(it.second));
            ++counter;
          } catch (const ConflictingKey &e) {
            // if key conflict exist, do nothing
            //          printf("detect conflict key, ignoring\n");
          }
        }
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

std::string DuplicateQuery::toString() {
  return "QUERY = DUPLICATE " + this->tableName() + "\"";
}
