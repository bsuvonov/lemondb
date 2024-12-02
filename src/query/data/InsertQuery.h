//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_INSERTQUERY_H
#define PROJECT_INSERTQUERY_H

#include <string>

#include "../Query.h"

class InsertQuery : public ComplexQuery {
  static constexpr const char *qname = "INSERT";

public:
  std::string query_name() override { return qname; }

  using ComplexQuery::ComplexQuery;

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }

  std::string toString() override;
};

#endif // PROJECT_INSERTQUERY_H
