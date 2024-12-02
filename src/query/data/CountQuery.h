#ifndef LEMONDB_SRC_QUERY_DATA_COUNTQUERY_H
#define LEMONDB_SRC_QUERY_DATA_COUNTQUERY_H

#include <string>

#include "../Query.h"

class CountQuery : public ComplexQuery {
  static constexpr const char *qname = "COUNT";

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
  // void processTableRows(Table &table, std::vector<int> &counts);
};

#endif // LEMONDB_SRC_QUERY_DATA_COUNTQUERY_H
