//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_QUITQUERY_H
#define PROJECT_QUITQUERY_H

#include <string>

#include "../Query.h"

class QuitQuery : public Query {
  static constexpr const char *qname = "QUIT";

public:
  QuitQuery() : Query("quittable") {}
  std::string query_name() override { return qname; }

  QueryResult::Ptr execute() override;
  bool is_desctructive() override { return true; }

  std::string toString() override;
};

#endif // PROJECT_QUITQUERY_H
