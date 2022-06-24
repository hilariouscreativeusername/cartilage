#pragma once

#include <mutex>
#include <deque>

namespace cartilage {

/*
 * Thread-safe queue class
 * Wrapper around a deque which does mutex locks to ensure no concurrent access
 */

template <typename T> class tsqueue {
public:
  tsqueue() = default;
  tsqueue(const tsqueue<T>&) = delete;
  virtual ~tsqueue() { clear(); }

public:
  const T& front() {
    std::scoped_lock lock(mutex_);
    return queue_.front();
  }

  const T& back() {
    std::scoped_lock lock(mutex_);
    return queue_.back();
  }

  void push_back(const T& t) {
    std::scoped_lock lock(mutex_);
    queue_.emplace_back(std::move(t));

		std::unique_lock<std::mutex> unique_lock(wait_mutex_);
		block_condition_.notify_one();
  }

  void push_front(const T& t) {
    std::scoped_lock lock(mutex_);
    queue_.emplace_front(std::move(t));

		std::unique_lock<std::mutex> unique_lock(wait_mutex_);
		block_condition_.notify_one();
  }

  T pop_back() {
    std::scoped_lock lock(mutex_);
    auto t = std::move(queue_.back());
    queue_.pop_back();
    return t;
  }

  T pop_front() {
    std::scoped_lock lock(mutex_);
    auto t = std::move(queue_.front());
    queue_.pop_front();
    return t;
  }

  size_t size() const {
    std::scoped_lock lock(mutex_);
    return queue_.size();
  }

  void clear() {
    std::scoped_lock lock(mutex_);
    queue_.clear();
  }

  bool empty() {
    std::scoped_lock lock(mutex_);
    return queue_.empty();
  }

  void wait() {
    while (empty()) {
      std::unique_lock<std::mutex> unique_lock(wait_mutex_);
      block_condition_.wait(unique_lock);
    }
  }

protected:
  std::mutex mutex_;
  std::deque<T> queue_;
  std::condition_variable block_condition_;
  std::mutex wait_mutex_;
};

}
