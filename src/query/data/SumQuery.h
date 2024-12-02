#ifndef LEMONDB_SRC_QUERY_DATA_SUMQUERY_H
#define LEMONDB_SRC_QUERY_DATA_SUMQUERY_H

#include <string>
#include <vector>

#include "../Query.h"

class SumQuery : public ComplexQuery {
  static constexpr const char *qname = "SUM";

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  size_t require_thread_num() override { return 1; }

  // TODO: combine()

  std::string toString() override;
  void processTableRows(Table *table, std::vector<int> *sums);
};

#endif // LEMONDB_SRC_QUERY_DATA_SUMQUERY_H
