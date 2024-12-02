#ifndef LEMONDB_SRC_QUERY_DATA_SWAPQUERY_H
#define LEMONDB_SRC_QUERY_DATA_SWAPQUERY_H

#include <string>

#include "../Query.h"

class SwapQuery : public ComplexQuery {
  static constexpr const char *qname = "SWAP";

public:
  using ComplexQuery::ComplexQuery;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }
  size_t require_thread_num() override { return 1; }

  std::string toString() override;
};

#endif // LEMONDB_SRC_QUERY_DATA_SWAPQUERY_H
