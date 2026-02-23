#include <condition_variable>
#include <functional>
#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>
#include <thread_pool/thread_pool.hpp>

ThreadPool::ThreadPool(size_t num_threads)
    : workers(), tasks(), queue_mutex(), mutex_condition(), stop(false) {
  for (size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back(std::thread(&ThreadPool::thread_loop, this));
  }
  spdlog::info("Thread pool initialized with {} threads.", num_threads);
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  mutex_condition.notify_all();
  for (std::thread &worker : workers) {
    worker.join();
  }
  workers.clear();
}

void ThreadPool::thread_loop() {
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      mutex_condition.wait(lock, [this] { return !tasks.empty() || stop; });
      if (stop && tasks.empty()) {
        return;
      }
      task = std::move(tasks.front());
      tasks.pop();
    };
    task();
  }
}

void ThreadPool::enqueue(const std::function<void()> &task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push(std::move(task));
  }
  mutex_condition.notify_one();
}
