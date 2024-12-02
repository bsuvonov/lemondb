//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_UPDATEQUERY_H
#define PROJECT_UPDATEQUERY_H

#include <string>

#include "../Query.h"

class UpdateQuery : public ComplexQuery {
  static constexpr const char *qname = "UPDATE";
  Table::ValueType
      fieldValue; // = (operands[0]=="KEY")? 0 :std::stoi(operands[1]);
  Table::FieldIndex fieldId;
  Table::KeyType keyValue;

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
};

#endif // PROJECT_UPDATEQUERY_H
