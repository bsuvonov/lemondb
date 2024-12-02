#ifndef PROJECT_TURNCATETABLEQUERY_H
#define PROJECT_TURNCATETABLEQUERY_H

#include <string>

#include "../Query.h"

class TruncateTableQuery : public Query {
  static constexpr const char *qname = "TRUNCATE";

public:
  using Query::Query;
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }

  std::string toString() override;
};

#endif
