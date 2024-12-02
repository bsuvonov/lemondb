#ifndef PROJECT_COPYTABLEQUERY_H
#define PROJECT_COPYTABLEQUERY_H

#include <memory>
#include <string>

#include "../../Manager.h"
#include "../Query.h"

class CopyTableQuery : public Query {
  static constexpr const char *qname = "COPYTABLE";
  const std::string newname;
  std::shared_ptr<Manager::Mana_sem> target;

public:
  CopyTableQuery(std::string originname, std::string newtablename);
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;

  std::string toString() override;
};
#endif
