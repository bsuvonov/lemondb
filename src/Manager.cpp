#include "Manager.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "query/QueryBuilders.h"
#include "query/QueryParser.h"
std::unique_ptr<Manager> Manager::ins = nullptr;

Manager &Manager::getInstance() {
  if (ins == nullptr)
    ins = std::unique_ptr<Manager>(new Manager);
  return *ins;
}
Manager::~Manager() {
  // for(auto &t:this->threads)
  //     t.join();
}

void Manager::set_threads(size_t th) {
  this->threads_sem = Pointered_sem(new Mana_sem((std::ptrdiff_t)th));
  // this->thread_num_lock.lock();
  this->threads_num = th;
  // this->thread_num_lock.unlock();
#ifdef TIME_ESTIMATE
  time_est.emplace("query_wait_time", std::chrono::milliseconds());
#endif
}
void Manager::start(std::istream &is) {
  this->is_end = false;
  this->query_counter = 0;
  this->read_end = false;
  // this->threads.emplace_back(read_query,is);
  this->threads.emplace_back(print_result);
  // puts("start read");
  read_query(is);
  while (!quit_flag && (listen_count--)) {
    listen_sem.acquire();
    listen_lock.lock();
    auto query_list = std::move(listened_queries.front());
    listened_queries.pop();
    listen_lock.unlock();
    read_query(query_list);
  }

  // puts("end read");
  this->read_end = true;
}
namespace {
inline static std::string extractQueryString(std::istream &is) {
  std::string buf;
  do {
    int const ch = is.get();
    if (ch == ';')
      return buf;
    if (ch == EOF)
      throw std::ios_base::failure("End of input");
    buf.push_back((char)ch);
  } while (true);
}
} // namespace

void Manager::add_query(const std::string &tablename, Query::Ptr query) {
  // if(tablename == "quittable"){
  //   this->quit();
  //   return;
  // }
  // if(tablename == "__listen_table"){
  //   listen_count_lock.lock();
  //   listen_count++;
  //   listen_count_lock.unlock();
  // }
  this->query_lock.lock();
  if (!this->table_query_map.contains(tablename)) {
    // puts("make new sem");
    this->table_query_map.emplace(tablename, Query_queue());
    this->table_query_sem.emplace(tablename, Pointered_sem(new Mana_sem(0)));
    this->threads.emplace_back(exeucte_query_for_table, std::string(tablename));
  }
  this->table_query_map[tablename].push(
      std::make_pair(query_counter, std::move(query)));
  this->query_lock.unlock();
  this->table_query_sem[tablename]->release();
}

void Manager::read_query(std::istream &is) {
  QueryParser p;

  p.registerQueryBuilder(std::make_unique<QueryBuilder(Debug)>());
  p.registerQueryBuilder(std::make_unique<QueryBuilder(Utils)>());
  p.registerQueryBuilder(std::make_unique<QueryBuilder(ManageTable)>());
  p.registerQueryBuilder(std::make_unique<QueryBuilder(Complex)>());

  while (is) {
    try {
      // puts("in loop");
      std::string const queryStr = extractQueryString(is);
      Query::Ptr query = p.parseQuery(queryStr);
      if (query->query_name() == "QUIT") {
        quit_flag = true;
        break;
      }
      if (query->query_name() == "LISTEN") {
        listen_count++;
      }
      auto tablename = query->tableName();

      ++query_counter;
      add_query(tablename, std::move(query));

    } catch (const std::ios_base::failure &e) {
      // End of input
      break;
    } catch (const std::exception &e) {
      std::cout.flush();
      std::cerr << e.what() << std::endl;
    }
  }
}

void Manager::read_query(const std::vector<std::string> &sv) {
  QueryParser p;

  p.registerQueryBuilder(std::make_unique<QueryBuilder(Debug)>());
  p.registerQueryBuilder(std::make_unique<QueryBuilder(Utils)>());
  p.registerQueryBuilder(std::make_unique<QueryBuilder(ManageTable)>());
  p.registerQueryBuilder(std::make_unique<QueryBuilder(Complex)>());

  for (auto &i : sv) {
    try {
      // remove the last ';'

      std::string queryStr(i);
      queryStr.pop_back();
      Query::Ptr query = p.parseQuery(queryStr);
      if (query->query_name() == "QUIT") {
        quit_flag = true;
        break;
      }
      if (query->query_name() == "LISTEN") {
        listen_count++;
      }
      auto tablename = query->tableName();

      ++query_counter;
      add_query(tablename, std::move(query));

    } catch (const std::ios_base::failure &e) {
      // End of input
      break;
    } catch (const std::exception &e) {
      std::cout.flush();
      std::cerr << e.what() << std::endl;
    }
  }
}
void Manager::quit() {
  puts("should not be called");
  exit(1);
  this->read_end = true;
  this->query_counter--;
}

void Manager::put_listen_result(std::vector<std::string> &&v) {
  listen_lock.lock();
  listened_queries.push(v);
  listen_lock.unlock();
  listen_sem.release();
}

size_t Manager::try_allocate_thread(size_t required_thread_num) {
  size_t j = 0;
  for (; j < required_thread_num; j++) {
    if (!ins->threads_sem->try_acquire()) {
      if (j != 0) {
        return j;
      } else {
        --j;
      }
    }
  }
  return j;
}

void Manager::single_threaded_query_execution(
    const std::string & /*table_name*/, size_t index, Query::Ptr q) {
  // ins->threads_sem->acquire();
  q->set_thread_num(try_allocate_thread(q->require_thread_num()));
#ifdef TIME_ESTIMATE
  auto start_time = std::chrono::high_resolution_clock::now();
#endif
  // auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(
  //    start_time.time_since_epoch());
  QueryResult::Ptr res;
  try {
    res = q->execute();
  } catch (EndWait &e) {
    ins->threads_sem->release((std::ptrdiff_t)q->get_thread_num());
    return;
  }
  ins->threads_sem->release((std::ptrdiff_t)q->get_thread_num());
#ifdef TIME_ESTIMATE
  auto end_time = std::chrono::high_resolution_clock::now();
#endif

  ins->result_lock.lock();
  ins->results.push(std::make_pair(index, std::move(res)));
  ins->result_lock.unlock();
  ins->result_sem.release();
#ifdef TIME_ESTIMATE
  auto print_ready_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  auto wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      print_ready_time - end_time);
  ins->time_est_lock.lock();
  if (!ins->time_est.contains(q->query_name())) {
    ins->time_est.emplace(q->query_name(), duration);
  } else {
    ins->time_est[q->query_name()] += duration;
  }
  ins->time_est["query_wait_time"] += wait_duration;
  ins->time_est_lock.unlock();
#endif
}

void Manager::exeucte_query_for_table(const std::string &s) {
  const std::string &table_name(s);
  std::vector<std::thread> table_threads{};
  while (!ins->is_end) {
    if (table_name.empty()) {
      puts("table name fail");
      break;
    }
    // puts(("start exeu "+table_name).data());
    ins->table_query_sem[table_name]->acquire();
    if (ins->is_end)
      break;
    ins->query_lock.lock();
    size_t const index = ins->table_query_map[table_name].front().first;
    Query::Ptr qptr(std::move(ins->table_query_map[table_name].front().second));
    ins->table_query_map[table_name].pop();
    ins->query_lock.unlock();

    if (qptr->is_desctructive()) {
      for (auto &t : table_threads) {
        t.join();
      }
      table_threads.clear();
      single_threaded_query_execution(table_name, index, std::move(qptr));
    } else {
      table_threads.emplace_back(single_threaded_query_execution, table_name,
                                 index, std::move(qptr));
    }
    // QueryResult::Ptr res;
    // try {
    //   res = qptr->execute();
    // } catch (EndWait &e) {
    //   continue;
    // }

    // ins->result_lock.lock();
    // ins->results.push(std::make_pair(index, std::move(res)));
    // ins->result_lock.unlock();
    // ins->result_sem.release();
  }
  for (auto &t : table_threads) {
    t.join();
  }
  table_threads.clear();
}

void Manager::print_result() {
  size_t print_index = 1;
  while (true) {
    // puts("printing");
    // std::cout <<(int) print_index << "\n";
    // printf("%zu\n",print_index);
    // puts(("printing ?"_f % print_index).data());

    ins->result_sem.acquire();
    ins->result_lock.lock();
    if (ins->results.top().first != print_index) {
      // puts("failing");
      // puts(("top at ?"_f % ins->results.top().first).data());
      // std::cout <<"fault"<<(int) ins->results.top().first << "\n";
      ins->result_lock.unlock();
      ins->result_sem.release();
      continue;
    }
    std::cout << print_index << "\n";
    QueryResult::Ptr res = (std::move(
        const_cast<QueryResult::Ptr &>((ins->results.top().second))));
    ins->results.pop();
    ins->result_lock.unlock();
    // if(res->is_listen()){

    //    auto &s = dynamic_cast<ListenResult &>(*res);
    //    auto listen_results = s.get_listen_result();
    //     Manager::getInstance().read_query(listen_results);
    //     ins->listen_count_lock.lock();
    //     ins->listen_count--;
    //     ins->listen_count_lock.unlock();
    // }

    if (res->success()) {
      if (res->display()) {
        std::cout << *res;
        std::cout.flush();
      } else {
#ifndef NDEBUG
        std::cout.flush();
        std::cerr << *res;
#endif
      }
    } else {
      std::cout.flush();
      std::cerr << "QUERY FAILED:\n\t" << *res;
    }

    // if(! ins->read_end) puts("still read");
    // if( print_index < ins->query_counter) puts(("pq not empty ? of counter"_f
    // % ins->query_counter).data()); if( print_index < ins->query_counter)
    // puts(("pq not empty ? in pq"_f % ins->results.size()).data());
    if (ins->read_end && print_index == ins->query_counter) {
      ins->is_end = true;
      for (auto &s : ins->table_query_sem) {
        s.second->release();
      }
      return;
    }
    ++print_index;
  }
}
