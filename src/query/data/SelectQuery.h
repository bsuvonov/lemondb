
#ifndef LEMONDB_SRC_QUERY_DATA_SELECTQUERY_H
#define LEMONDB_SRC_QUERY_DATA_SELECTQUERY_H

#include <string>
#include <vector>

#include "../Query.h"

class SelectQuery : public ComplexQuery {
  static constexpr const char *qname = "SELECT";

  std::vector<std::vector<std::string>> data_result;

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  size_t require_thread_num() override { return 1; }
  std::string toString() override;

private:
  void select_part_helper(std::vector<std::vector<std::string>> *result,
                          const Table::Iterator &start,
                          const Table::Iterator &end,
                          const std::vector<Table::FieldIndex> &filed_id_list,
                          int id, size_t offset);
};

#endif // LEMONDB_SRC_QUERY_DATA_SELECTQUERY_H
