//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_DUMPTABLEQUERY_H
#define PROJECT_DUMPTABLEQUERY_H

#include <string>
#include <utility>

#include "../Query.h"

class DumpTableQuery : public Query {
  static constexpr const char *qname = "DUMP";
  const std::string fileName;

public:
  DumpTableQuery(std::string table, std::string filename)
      : Query(std::move(table)), fileName(std::move(filename)) {}
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;

  std::string toString() override;
};

#endif // PROJECT_DUMPTABLEQUERY_H
