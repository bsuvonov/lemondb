
#include "WaitQuery.h"

QueryResult::Ptr WaitQuery::execute() {
  // puts("start wait");
  this->target->acquire();
  // puts("finish wait");
  throw EndWait();
  // try {
  //   Database &db = Database::getInstance();
  //   auto &ori = db[this->tableName()];
  //   auto ntb = std::make_unique<Table>(this->newname, ori);
  //   db.registerTable(std::move(ntb));
  //   return std::make_unique<SuccessMsgResult>(qname);
  // } catch (DuplicatedTableName &e) {
  //   return std::make_unique<ErrorMsgResult>(qname, tableName(),
  //                                           "duplicate table name");
  // }
  return std::make_unique<NullQueryResult>();
}

std::string WaitQuery::toString() {
  return "QUERY = WAIT, FROM Table = \"" + tableName() + "\"";
}
