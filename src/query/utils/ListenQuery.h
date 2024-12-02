#ifndef LEMONDB_SRC_QUERY_DATA_LISTENQUERY_H
#define LEMONDB_SRC_QUERY_DATA_LISTENQUERY_H

#include <string>
#include <utility>

#include "../Query.h"

class ListenQuery : public Query {
  static constexpr const char *qname = "LISTEN";
  const std::string fileName;

public:
  explicit ListenQuery(std::string filename)
      // table name set to __listen_table, for reserved usage
      : Query("__listen_table"), fileName(std::move(filename)) {}
  std::string query_name() override { return qname; }

  bool is_desctructive() override { return true; }

  QueryResult::Ptr execute() override;

  std::string toString() override;
};

#endif // LEMONDB_SRC_QUERY_DATA_LISTENQUERY_H
