#ifndef PROJECT_DELETEQUERY_H
#define PROJECT_DELETEQUERY_H

#include <string>

#include "../Query.h"

class DeleteQuery : public ComplexQuery {
  static constexpr const char *qname = "DELETE";

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }

  std::string toString() override;
};

#endif // PROJECT_DELETEQUERY_H
