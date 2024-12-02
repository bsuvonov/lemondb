#ifndef PROJECT_WAITQUERY_H
#define PROJECT_WAITQUERY_H

#include <memory>
#include <string>
#include <utility>

#include "../../Manager.h"
#include "../Query.h"
class WaitQuery : public Query {
  static constexpr const char *qname = "WAIT";
  std::shared_ptr<Manager::Mana_sem> target;
  //   const std::string newname;

public:
  explicit WaitQuery(std::string originname,
                     std::shared_ptr<Manager::Mana_sem> t)
      : Query(std::move(originname)), target(std::move(t)) {}
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }

  std::string toString() override;
};
#endif
