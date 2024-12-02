#ifndef PROJECT_SUBQUERY_H
#define PROJECT_SUBQUERY_H

#include <string>
#include <vector>

#include "../Query.h"

class SubQuery : public ComplexQuery {
  static constexpr const char *qname = "SUB";

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
};

#endif // PROJECT_SUBQUERY_H
