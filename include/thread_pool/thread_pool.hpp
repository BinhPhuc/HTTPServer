#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable mutex_condition;
  bool stop;
  void thread_loop();

public:
  ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
  ~ThreadPool();
  void enqueue(const std::function<void()> &task);
};
