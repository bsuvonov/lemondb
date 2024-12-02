#include "SelectQuery.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

#include "../../db/Database.h"
// select_part_helper is a helper function to select the part of the data
// and store it in the data_result
// from [start to end)
void SelectQuery::select_part_helper(
    std::vector<std::vector<std::string>> *result, const Table::Iterator &start,
    const Table::Iterator &end,
    const std::vector<Table::FieldIndex> &filed_id_list, int /*id*/,
    size_t offset) {

  size_t index = offset;
  for (auto it = start; it != end; ++it) {
    if (this->evalCondition(*it)) {
      std::vector<std::string> line_result(filed_id_list.size() + 1);
      line_result[0] = (it->key());
      std::size_t i = 1;
      for (auto field_id : filed_id_list) {
        line_result[i] = (std::to_string((*it)[field_id]));
        ++i;
      }
      (*result)[index++] = std::move(line_result);
    }
  }

  //  auto end_time = std::chrono::high_resolution_clock::now();
  //  auto duration =
  //  std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
  //  start_time); std::cout << "Thread [" << id << "] execution time: " <<
  //  duration.count() << " ms" << std::endl;
}
QueryResult::Ptr SelectQuery::execute() {
  //  auto total_start_time = std::chrono::high_resolution_clock::now(); //
  //  total time start
  if (this->getOperands().empty())
    return make_unique<ErrorMsgResult>(qname, this->tableName().c_str(),
                                       "No operand (? operands)."_f %
                                           getOperands().size());
  // KEY should be in the first position in field
  if (this->getOperands()[0] != "KEY")
    return std::make_unique<ErrorMsgResult>(
        qname, this->tableName().c_str(),
        "SELECT field KEY not in the first position.");
  Database &db = Database::getInstance();

  std::vector<Table::FieldIndex> filed_id_list(this->getOperands().size() - 1);

  try {
    auto &table = db[this->tableName()];
    //    auto init_start_time = std::chrono::high_resolution_clock::now();
    auto result = initCondition(table);
    //    auto init_end_time = std::chrono::high_resolution_clock::now();
    //    std::cout << "Condition initialization time: "
    //              <<
    //              std::chrono::duration_cast<std::chrono::milliseconds>(init_end_time
    //              - init_start_time).count()
    //              << " ms" << std::endl;
    for (std::size_t j = 1; j < this->getOperands().size(); ++j) {
      filed_id_list[j - 1] = (table.getFieldIndex(this->getOperands()[j]));
    }

    if (result.second) {
      //      thread_num = 2; // thread num = 2 is maybe the best choice
      size_t const table_size = table.size();
      size_t const step = table_size / get_thread_num();

      this->data_result.resize(table_size);

      if (get_thread_num() == 1 || this->getCondition().empty()) {
        this->select_part_helper(&(this->data_result), table.begin(),
                                 table.end(), filed_id_list, 0, 0);
      } else {
        std::vector<std::thread> threads;
        //        auto multi_thread_start =
        //        std::chrono::high_resolution_clock::now(); // multi thread
        //        start time
        for (size_t i = 0; i < get_thread_num(); ++i) {
          size_t const start_index = i * step;
          size_t const end_index =
              (i == get_thread_num() - 1) ? table_size : start_index + step;

          threads.emplace_back(
              [this, &table, start_index, end_index, &filed_id_list, i]() {
                this->select_part_helper(&(this->data_result),
                                         table.begin() + (int)start_index,
                                         table.begin() + (int)end_index,
                                         filed_id_list, (int)i, start_index);
              });
        }

        for (auto &t : threads) {
          t.join();
        }
        //        auto multi_thread_end =
        //        std::chrono::high_resolution_clock::now(); // multi thread
        //        time end std::cout << "Multi-thread execution time: "
        //                  <<
        //                  std::chrono::duration_cast<std::chrono::milliseconds>(multi_thread_end
        //                  - multi_thread_start).count()
        //                  << " ms" << std::endl;
      }
    }

    //    auto clean_start_time = std::chrono::high_resolution_clock::now(); //
    //    clean time start
    data_result.erase(std::remove_if(data_result.begin(), data_result.end(),
                                     [](const std::vector<std::string> &row) {
                                       return row.empty();
                                     }),
                      data_result.end());
    //    auto clean_end_time = std::chrono::high_resolution_clock::now();  //
    //    clean time stop std::cout << "Result cleaning time: "
    //              <<
    //              std::chrono::duration_cast<std::chrono::milliseconds>(clean_end_time
    //              - clean_start_time).count()
    //              << " ms" << std::endl;

    //    auto sort_start_time = std::chrono::high_resolution_clock::now(); //
    //    sort time start
    std::sort(data_result.begin(), data_result.end(),
              [](const std::vector<std::string> &a,
                 const std::vector<std::string> &b) { return a[0] < b[0]; });

    return std::make_unique<DataResult>(data_result);
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
std::string SelectQuery::toString() {
  return "QUERY = SELECT " + this->tableName() + "\"";
}
