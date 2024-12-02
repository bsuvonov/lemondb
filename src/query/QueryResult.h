//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_QUERYRESULT_H
#define PROJECT_QUERYRESULT_H

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../utils/formatter.h"

class QueryResult {
public:
  typedef std::unique_ptr<QueryResult> Ptr;

  virtual bool success() = 0;

  virtual bool display() = 0;

  virtual ~QueryResult() = default;

  virtual bool is_listen() { return false; }

  friend std::ostream &operator<<(std::ostream &os, const QueryResult &table);

protected:
  virtual std::ostream &output(std::ostream &os) const = 0;
};

class FailedQueryResult : public QueryResult {
  const std::string data;

public:
  bool success() override { return false; }

  bool display() override { return false; }
};

class SucceededQueryResult : public QueryResult {
public:
  bool success() override { return true; }
};

class NullQueryResult : public SucceededQueryResult {
public:
  bool display() override { return false; }

protected:
  std::ostream &output(std::ostream &os) const override { return os; }
};

class ErrorMsgResult : public FailedQueryResult {
  std::string msg;

public:
  bool display() override { return false; }

  ErrorMsgResult(const char *qname, const std::string &msg) {
    this->msg = R"(Query "?" failed : ?)"_f % qname % msg;
  }

  ErrorMsgResult(const char *qname, const std::string &table,
                 const std::string &msg) {
    this->msg = R"(Query "?" failed in Table "?" : ?)"_f % qname % table % msg;
  }

protected:
  std::ostream &output(std::ostream &os) const override {
    return os << msg << "\n";
  }
};

class SuccessMsgResult : public SucceededQueryResult {
  std::string msg;
  bool debug_ = true;

public:
  bool display() override { return debug_; }

  explicit SuccessMsgResult(const int number, bool debug = true)
      : debug_(debug) {
    this->msg = R"(ANSWER = ?)"_f % number;
  }

  explicit SuccessMsgResult(const std::vector<int> &results, bool debug = true)
      : debug_(debug) {
    std::stringstream ss;
    ss << "ANSWER = ( ";
    for (auto result : results) {
      ss << result << " ";
    }
    ss << ")";
    this->msg = ss.str();
  }

  explicit SuccessMsgResult(const char *qname, bool debug = false)
      : debug_(debug) {
    this->msg = R"(Query "?" success.)"_f % qname;
  }

  SuccessMsgResult(const char *qname, const std::string &msg,
                   bool debug = false)
      : debug_(debug) {
    this->msg = R"(Query "?" success : ?)"_f % qname % msg;
  }

  SuccessMsgResult(const char *qname, const std::string &table,
                   const std::string &msg, bool debug = false)
      : debug_(debug) {
    this->msg = R"(Query "?" success in Table "?" : ?)"_f % qname % table % msg;
  }

protected:
  std::ostream &output(std::ostream &os) const override {
    if (debug_) {
      return os << msg << "\n";
    }
    return os;
  }
};

class RecordCountResult : public SucceededQueryResult {
  const int affectedRows;

public:
  bool display() override { return true; }

  explicit RecordCountResult(int count) : affectedRows(count) {}

protected:
  std::ostream &output(std::ostream &os) const override {
    return os << "Affected ? rows."_f % affectedRows << "\n";
  }
};

class DataResult : public SucceededQueryResult {
  const std::vector<std::vector<std::string>> data_result;

public:
  bool display() override { return true; }

  explicit DataResult(std::vector<std::vector<std::string>> result)
      : data_result(std::move(result)) {}

protected:
  std::ostream &output(std::ostream &os) const override {
    for (const auto &r : data_result) {
      os << "( ";
      for (const auto &s : r) {
        os << s << " ";
      }
      os << ")\n";
    }
    os.flush();
    return os;
  }
};

class ListenResult : public SucceededQueryResult {
  // const std::vector<std::string> listen_result;
  const std::string listen_name;

public:
  bool display() override { return true; }

  explicit ListenResult(std::string name) : listen_name(std::move(name)) {}

  bool is_listen() override { return true; }

  // std::vector<std::string> get_listen_result(){
  //   return listen_result;
  // }

protected:
  std::ostream &output(std::ostream &os) const override {
    os << "ANSWER = ( listening from " << listen_name << " )\n";
    return os;
  }
};

#endif // PROJECT_QUERYRESULT_H
