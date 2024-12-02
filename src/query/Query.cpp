//
// Created by liu on 18-10-25.
//

#include "Query.h"

#include <cassert>
#include <thread>
#include <unordered_map>
#include <utility>

std::pair<std::string, bool> ComplexQuery::initCondition(const Table &table) {
  const std::unordered_map<std::string, int> opmap{
      {">", '>'}, {"<", '<'}, {"=", '='}, {">=", 'g'}, {"<=", 'l'},
  };
  std::pair<std::string, bool> result = {"", true};
  for (auto &cond : condition) {
    if (cond.field == "KEY") {
      if (cond.op != "=") {
        throw IllFormedQueryCondition("Can only compare equivalence on KEY");
      } else if (result.first.empty()) {
        result.first = cond.value;
      } else if (result.first != cond.value) {
        result.second = false;
        return result;
      }
    } else {
      cond.fieldId = table.getFieldIndex(cond.field);
      cond.valueParsed =
          (Table::ValueType)std::strtol(cond.value.c_str(), nullptr, 10);
      int op = 0;
      try {
        op = opmap.at(cond.op);
      } catch (const std::out_of_range &e) {
        throw IllFormedQueryCondition(
            R"("?" is not a valid condition operator.)"_f % cond.op);
      }
      switch (op) {
      case '>':
        cond.comp = std::greater<>();
        break;
      case '<':
        cond.comp = std::less<>();
        break;
      case '=':
        cond.comp = std::equal_to<>();
        break;
      case 'g':
        cond.comp = std::greater_equal<>();
        break;
      case 'l':
        cond.comp = std::less_equal<>();
        break;
      default:
        assert(0); // should never be here
      }
    }
  }
  return result;
}

bool ComplexQuery::evalCondition(const Table::Object &object) {
  bool ret = true;
  for (const auto &cond : condition) {
    if (cond.field != "KEY") {
      ret = ret && cond.comp(object[cond.fieldId], cond.valueParsed);
    } else {
      ret = ret && (object.key() == cond.value);
    }
  }
  return ret;
}

bool ComplexQuery::testKeyCondition(
    Table *table,
    const std::function<void(bool, Table::Object::Ptr &&)> &function) {
  auto condResult = initCondition(*table);
  if (!condResult.second) {
    function(false, nullptr);
    return true;
  }
  if (!condResult.first.empty()) {
    auto object = (*table)[condResult.first];
    if (object != nullptr && evalCondition(*object)) {
      function(true, std::move(object));
    } else {
      function(false, nullptr);
    }
    return true;
  }
  return false;
}

void ComplexQuery::parallel_operate(const std::function<operation_on_row> &op,
                                    std::vector<void *> datas, Table *table) {
  if (get_thread_num() == 1) {
    for (auto it : (*table)) {
      if (this->evalCondition(it)) {
        op(it, datas[0]);
      }
    }
    return;
  }
  std::vector<std::thread> jobs{};
  size_t const split_size = (*table).size() / get_thread_num();

  for (size_t i = 1; i < get_thread_num(); ++i) {
    jobs.emplace_back([this, i, &table, split_size, op, &datas]() {
      for (auto it = (*table).begin() + int((i - 1) * split_size);
           it != (*table).begin() + int((i)*split_size); it++) {
        if (this->evalCondition(*it)) {
          op(*it, datas[i]);
        }
      }
    });
  }
  jobs.emplace_back([this, &table, split_size, op, &datas]() {
    for (auto it = (*table).begin() + int((get_thread_num() - 1) * split_size);
         it != (*table).end(); it++) {
      if (this->evalCondition(*it)) {
        op(*it, datas[0]);
      }
    }
  });
  for (auto &j : jobs) {
    j.join();
  }
}
