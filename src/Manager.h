#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Manager_sem.h"
#include "query/Query.h"
// #define TIME_ESTIMATE //enable time measure feature
#define MAX_QUERY_QUEUE_SIZE 10000000
class Manager {
public:
  typedef std::pair<size_t, Query::Ptr>
      Query_with_index; // indexed unexected query
  typedef std::pair<size_t, QueryResult::Ptr>
      Result_with_index; // indexed result
  typedef std::queue<Query_with_index> Query_queue;
  struct ResultCompare {
    bool operator()(const Result_with_index &a,
                    const Result_with_index &b) const {
      return a.first > b.first;
    }
  };
  typedef std::counting_semaphore<MAX_QUERY_QUEUE_SIZE> Mana_sem;
  typedef std::unique_ptr<Mana_sem> Pointered_sem;

private:
  static std::unique_ptr<Manager> ins; // singleton pointer
#ifdef TIME_ESTIMATE
  std::mutex time_est_lock;
  std::unordered_map<std::string, std::chrono::milliseconds>
      time_est; // to estimate time for query
#endif

  size_t threads_num{};
  Pointered_sem threads_sem;

  size_t query_counter{};
  // size_t total_query;
  bool read_end{};
  bool is_end{};
  // map to spilt different query into tables
  std::mutex query_lock;
  std::unordered_map<std::string, Query_queue> table_query_map;
  std::unordered_map<std::string, Pointered_sem> table_query_sem;
  // vector for storing other threads
  std::vector<std::thread> threads;

  // pqueue for efficient output result
  std::mutex result_lock;
  std::priority_queue<Result_with_index, std::vector<Result_with_index>,
                      ResultCompare>
      results;
  Mana_sem result_sem;

  std::mutex listen_lock;
  std::queue<std::vector<std::string>> listened_queries;
  Mana_sem listen_sem;
  size_t listen_count = 0;
  bool quit_flag = false;

  // std::vector<size_t> listen_barrier{}; // barrier for listen query,
  // before the barrier is reached, the queries after the barrier will not be
  // executed

  static void single_threaded_query_execution(const std::string &table_name,
                                              size_t index, Query::Ptr q);
  static void exeucte_query_for_table(const std::string &table_name);
  static void print_result();
  void read_query(std::istream &is);
  void read_query(const std::vector<std::string> &sv);
  Manager() : result_sem(Mana_sem(0)), listen_sem(Mana_sem(0)) {
    // thread_states.reserve(threads_num);
    // for (size_t i = 0; i < threads_num; i++) {
    //   thread_states.emplace_back(i);
    // }
  }
  static size_t try_allocate_thread(size_t required_thread_num);

public:
  static Manager &getInstance();
  void set_threads(size_t th);
  void start(std::istream &is);
  void add_query(const std::string &tablename, Query::Ptr query);
  void end() {
    for (auto &t : threads)
      t.join();
    threads.clear();

#ifdef TIME_ESTIMATE
    int64_t total = 0;
    for (auto &it : time_est) {
      total += it.second.count();
      std::cout << it.first << " cost:" << it.second.count() << " ms\n";
    }
    std::cout << "all execution time: " << total << "ms\n";
#endif
  }
  void put_listen_result(std::vector<std::string> &&v);
  void quit();
  ~Manager();
};

#endif // PROJECT_MANAGER_H
