#ifndef PROJECT_ADDQUERY_H
#define PROJECT_ADDQUERY_H

#include <string>

#include "../Query.h"

class AddQuery : public ComplexQuery {
  static constexpr const char *qname = "ADD";

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
};

#endif // PROJECT_ADDQUERY_H
