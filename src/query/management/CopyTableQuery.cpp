
#include "CopyTableQuery.h"

#include <memory>
#include <string>
#include <utility>

#include "../../db/Database.h"
#include "WaitQuery.h"
// constexpr const char *CopyTableQuery::qname;

CopyTableQuery::CopyTableQuery(std::string originname, std::string newtablename)
    : Query(std::move(originname)), newname(std::move(newtablename)),
      target(std::make_shared<Manager::Mana_sem>(0)) {
  Manager::getInstance().add_query(
      newname, std::make_unique<WaitQuery>(this->tableName(), target));
}

QueryResult::Ptr CopyTableQuery::execute() {
  try {
    Database &db = Database::getInstance();
    auto &ori = db[this->tableName()];
    auto ntb = std::make_unique<Table>(this->newname, ori);
    db.registerTable(std::move(ntb));
    // puts("tend to finish wait");
    target->release();
    return std::make_unique<SuccessMsgResult>(qname, tableName());
  } catch (DuplicatedTableName &e) {
    return std::make_unique<ErrorMsgResult>(qname, newname,
                                            "duplicate table name");
    target->release();
  }
}

std::string CopyTableQuery::toString() {
  return "QUERY = COPYTABLE, FROM Table = \"" + tableName() + "\" TO \"" +
         newname + "\"";
}
