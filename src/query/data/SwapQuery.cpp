
#include "SwapQuery.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../db/Database.h"

QueryResult::Ptr SwapQuery::execute() {
  if (this->getOperands().size() != 2)
    return make_unique<ErrorMsgResult>(
        qname, this->tableName().c_str(),
        "Should be 2 operands (provided ? operands)."_f % getOperands().size());

  // if the two getOperands() are the same, do nothing
  // if (this->getOperands()[0] == this->getOperands()[1])
  //   return make_unique<RecordCountResult>(0);

  Database &db = Database::getInstance();

  Table::SizeType counter = 0;

  try {
    auto &table = db[this->tableName()];
    Table::FieldIndex const two_index[2] = {
        table.getFieldIndex(this->getOperands()[0]),
        table.getFieldIndex(this->getOperands()[1])};
    auto result = initCondition(table);

    auto op = [&two_index](Table::Object object, void *data) {
      std::swap(object[two_index[0]], object[two_index[1]]);
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

      // for (auto it : table) {
      //   if (this->evalCondition(it)) {
      //     std::swap(it[two_index[0]], it[two_index[1]]);
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
std::string SwapQuery::toString() {
  return "QUERY = SWAP " + this->tableName() + "\"";
}
