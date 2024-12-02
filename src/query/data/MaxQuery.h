
#ifndef LEMONDB_SRC_QUERY_DATA_MAXQUERY_H
#define LEMONDB_SRC_QUERY_DATA_MAXQUERY_H

#include <string>
#include <vector>

#include "../Query.h"

class MaxQuery : public ComplexQuery {
  static constexpr const char *qname = "MAX";
  bool find = false;

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
  std::vector<int> find_max(const std::vector<Table::FieldIndex> &field_id_list,
                            const Table::Iterator &start,
                            const Table::Iterator &end);
};

#endif // LEMONDB_SRC_QUERY_DATA_MAXQUERY_H
