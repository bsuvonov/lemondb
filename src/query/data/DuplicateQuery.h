#ifndef LEMONDB_SRC_QUERY_DATA_DUPLICATEQUERY_H
#define LEMONDB_SRC_QUERY_DATA_DUPLICATEQUERY_H

#include <string>
#include <utility>
#include <vector>

#include "../Query.h"

class DuplicateQuery : public ComplexQuery {
  static constexpr const char *qname = "DUPLICATE";

public:
  std::string query_name() override { return qname; }

  using ComplexQuery::ComplexQuery;
  typedef std::vector<Table::ValueType> row_type;
  typedef std::vector<std::pair<std::string, row_type>> record_type;

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
};

#endif // LEMONDB_SRC_QUERY_DATA_DUPLICATEQUERY_H
