#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <functional>
#include <queue>
#include <memory>
using namespace std;
class ThreadPool {
public:
	ThreadPool(int threadnums);
	template<class T, class ...Args>
	auto enqueue(T&& t,Args&&...args) -> future<typename result_of<T(Args...)>::type>;
	void worker();
	~ThreadPool();
private:
	queue<function<void()>> que;
	vector<thread>threads;
	bool isstop;
	mutex mtx;
	condition_variable cv;
};

ThreadPool::ThreadPool(int threadnums) : isstop{false} {
	for (int i = 0; i < threadnums; i++)
	{
		threads.emplace_back([this]() {this->worker(); });
	}
}
template<class T,class ...Args>
auto ThreadPool::enqueue(T &&t, Args&&...args) -> future<typename result_of<T(Args...)>::type>
{
	using functype = typename result_of<T(Args...)>::type;

	auto task = make_shared<std::packaged_task<functype()>>(
		bind(forward<T>(t),forward<Args>(args)...)
	);
	future<functype> rsfu = task->get_future();
	{
		lock_guard<mutex> lockguard(this->mtx);
		if (isstop)
			throw runtime_error("´íÎó£ºÏß³Ì³ØÒÑÍ£Ö¹");
		que.emplace([task]() {
			(*task)();
			});
	}
	cv.notify_one();
	return rsfu;
}
auto ThreadPool::worker()->void
{
	while (true)
	{
		function<void()> task;
		{
			unique_lock<mutex> lock(mtx);
			cv.wait(lock, [this] {
				return this->isstop || !this->que.empty();
				});
			if (isstop && que.empty()) return;
			task = move(this->que.front());
			this->que.pop();

		}
		task();
	}
}
ThreadPool::~ThreadPool()
{
	{
		unique_lock<mutex>(mtx);
		isstop = true;
	}
	cv.notify_all();

	for (thread& thr : threads)
	{
		thr.join();
	}
}

