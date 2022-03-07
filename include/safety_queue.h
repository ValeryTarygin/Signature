#pragma once
#include <iostream>
#include <mutex>
#include <queue>
#include <memory>

namespace thread_utils {

template <typename T>
class ThreadSafetyQueue
{
public:
	ThreadSafetyQueue() = default;
	ThreadSafetyQueue(const ThreadSafetyQueue& other)
	{
		std::lock_guard lck(other.mutex);
		queue = other.queue;
	}

	ThreadSafetyQueue& operator= (const ThreadSafetyQueue& other)
	{
		std::lock_guard lck(other.mutex);
		queue = other.queue;
	}

	void push(const T& val)
	{
		std::lock_guard lck(mutex);
		queue.push(val);
		cv.notify_one();
	}

	void wait_or_pop(T& val)
	{
		std::unique_lock lck(mutex);
		cv.wait(lck, [this]() { return !queue.empty(); });
		val = queue.front();
		queue.pop();
	}

	std::shared_ptr<T> wait_or_pop()
	{
		std::unique_lock lck(mutex);
		cv.wait(lck, [this]() { return !queue.empty(); });
		std::shared_ptr<T> ret = std::make_shared<T>(queue.front());
		queue.pop();
		return ret;
	}

	std::shared_ptr<T> try_pop()
	{
		std::lock_guard lck(mutex);
		if (queue.empty())
			return std::make_shared<T>();
		std::shared_ptr<T> ret = std::make_shared<T>(queue.front());
		queue.pop();
		return ret;
	}

	bool try_pop(T& val)
	{
		std::lock_guard lck(mutex);
		if (queue.empty())
			return false;
		val = queue.front();
		queue.pop();
		return true;
	}

	bool empty() const noexcept
	{
		std::lock_guard lck(mutex);
		return queue.empty();
	}

	size_t size() const noexcept
	{
		std::lock_guard lck(mutex);
		return queue.size();
	}

private:
	mutable std::mutex mutex;
	std::queue<T> queue;
	std::condition_variable cv;
};

} // thread_utils