//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_QUERY_H
#define PROJECT_QUERY_H

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../db/Table.h"
#include "QueryResult.h"

struct QueryCondition {
  std::string field;
  size_t fieldId;
  std::string op;
  std::function<bool(const Table::ValueType &, const Table::ValueType &)> comp;
  std::string value;
  Table::ValueType valueParsed;
};

class Query {
  std::string targetTable;
  // int id = -1;

  size_t thread_num = 1;

public:
  Query() = default;

  explicit Query(std::string targetTable)
      : targetTable(std::move(targetTable)) {}

  typedef std::unique_ptr<Query> Ptr;

  const std::string &tableName() const { return targetTable; }

  virtual QueryResult::Ptr execute() = 0;

  virtual std::string toString() = 0;

  virtual std::string query_name() = 0;

  virtual bool is_desctructive() { return false; }

  virtual size_t require_thread_num() { return 1; }

  virtual void set_thread_num(size_t tn) { this->thread_num = tn; }

  virtual size_t get_thread_num() { return this->thread_num; }

  virtual ~Query() = default;
};
class EndWait {};
class NopQuery : public Query {
public:
  QueryResult::Ptr execute() override {
    return std::make_unique<NullQueryResult>();
  }
  // bool is_desctructive(){return true;}
  std::string query_name() override { return "NOOP"; }

  std::string toString() override { return "QUERY = NOOP"; }
};

class ComplexQuery : public Query {
  /** The field names in the first () */
  std::vector<std::string> operands;
  /** The function used in where clause */
  std::vector<QueryCondition> condition;

public:
  typedef std::unique_ptr<ComplexQuery> Ptr;
  typedef void(operation_on_row)(Table::Object object, void *data);
  /**
   * init a fast condition according to the table
   * note that the condition is only effective if the table fields are not
   * changed
   * @param table
   * @param conditions
   * @return a pair of the key and a flag
   * if flag is false, the condition is always false
   * in this situation, the condition may not be fully initialized to save time
   */
  std::pair<std::string, bool> initCondition(const Table &table);

  /**
   * skip the evaluation of KEY
   * (which should be done after initConditionFast is called)
   * @param conditions
   * @param object
   * @return
   */
  bool evalCondition(const Table::Object &object);

  /**
   * This function seems have small effect and causes somme bugs
   * so it is not used actually
   * @param table
   * @param function
   * @return
   */
  bool testKeyCondition(
      Table *table,
      const std::function<void(bool, Table::Object::Ptr &&)> &function);

  ComplexQuery(std::string targetTable, std::vector<std::string> operands,
               std::vector<QueryCondition> condition)
      : Query(std::move(targetTable)), operands(std::move(operands)),
        condition(std::move(condition)) {}

  /** Get operands in the query */
  const std::vector<std::string> &getOperands() const { return operands; }

  /** Get condition in the query, seems no use now */
  const std::vector<QueryCondition> &getCondition() { return condition; }

  // pre_defined functions to perform multithread operations
  // thread num should be passed by Query
  // data.size() should correspond to thread_num
  // !No init here, should be called before
  void parallel_operate(const std::function<operation_on_row> &op,
                        std::vector<void *> datas, Table *table);
};

#endif // PROJECT_QUERY_H
