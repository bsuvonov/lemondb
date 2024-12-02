//
// Created by liu on 18-10-25.
//

#include "PrintTableQuery.h"

#include <iostream>
#include <memory>
#include <string>

#include "../../db/Database.h"

// constexpr const char *PrintTableQuery::qname;

QueryResult::Ptr PrintTableQuery::execute() {
  Database &db = Database::getInstance();
  try {
    auto &table = db[this->tableName()];
    std::cout << "================\n";
    std::cout << "TABLE = ";
    std::cout << table;
    std::cout << "================\n" << std::endl;
    return std::make_unique<SuccessMsgResult>(qname, this->tableName());
  } catch (const TableNameNotFound &e) {
    return std::make_unique<ErrorMsgResult>(qname, this->tableName(),
                                            "No such table.");
  }
}

std::string PrintTableQuery::toString() {
  return "QUERY = SHOWTABLE, Table = \"" + this->tableName() + "\"";
}