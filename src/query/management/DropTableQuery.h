//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_DROPTABLEQUERY_H
#define PROJECT_DROPTABLEQUERY_H

#include <string>

#include "../Query.h"

class DropTableQuery : public Query {
  static constexpr const char *qname = "DROP";

public:
  using Query::Query;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }

  std::string toString() override;
};

#endif // PROJECT_DROPTABLEQUERY_H
