//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_LISTTABLEQUERY_H
#define PROJECT_LISTTABLEQUERY_H

#include <string>

#include "../Query.h"

class ListTableQuery : public Query {
  static constexpr const char *qname = "LIST";

public:
  QueryResult::Ptr execute() override;
  std::string query_name() override { return qname; }

  std::string toString() override;
};

#endif // PROJECT_LISTTABLEQUERY_H
