#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <future>
#include <memory>
#include "safety_queue.h"

namespace thread_utils {

class JoinerPool final
{
public:
	explicit JoinerPool(std::vector<std::thread>& pool) : refPool(pool)
	{}

	~JoinerPool()
	{
		for (auto& t : refPool)
		{
			if (t.joinable())
				t.join();
		}
	}

private:
	std::vector<std::thread>& refPool;
};

class ThreadPool final
{
	using QueueTasks = ThreadSafetyQueue<std::function<void()>>;
public:
	ThreadPool() : joiner(pool), numThread(std::thread::hardware_concurrency())
	{
		pool.resize(numThread);
		Run();
	}

	void Run()
	{
		for (auto& t : pool)
			t = std::thread(&ThreadPool::ThreadFunc, this);
	}

	unsigned int GetNumThreads() const noexcept
	{
		return numThread;
	}

	size_t GetQueueSize() const noexcept
	{
		return queue.size();
	}

	~ThreadPool()
	{
		isExit = true;
		cv.notify_all();
	}

	template<typename F, typename... Args>
	auto Submit(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
	{
		using TypeResult = std::invoke_result_t<F, Args...>;

		auto task = std::make_shared<std::packaged_task<TypeResult()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));

		std::future<TypeResult> future = task->get_future();
		{
			queue.push([t = std::move(task)] {
					(*t)();
				});
		}

		cv.notify_one();

		return future;
	}

private:
	void ThreadFunc()
	{
		while (true)
		{
#if 1 // with condition_variable
			std::unique_lock lck(cvMutex);
			cv.wait(lck, [this] { return isExit || !queue.empty(); });

			if (isExit)
				return;

			std::function<void()> task;
			if(queue.try_pop(task))
				task();
#endif
#if 0 // without condition_variable
			std::function<void()> task;
			if (queue.try_pop(task))
			{
				task();
			}
			else
			{
				std::this_thread::yield();
			}
#endif
		}
	}

private:
	QueueTasks queue;
	std::vector<std::thread> pool;
	JoinerPool joiner;

	unsigned int numThread{0};
	bool isExit{false};
	mutable std::mutex cvMutex;
	std::condition_variable cv;
};

} // thread_utils