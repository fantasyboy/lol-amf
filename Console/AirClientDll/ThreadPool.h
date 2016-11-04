#ifndef _ThreadPool_2015_12_2_38_H_
#define _ThreadPool_2015_12_2_38_H_

/*
* We must detach threads to prevent crashes when the process suddlenly close.
* If we use static global s_instance, the std::thread will not return.
* So, you should call getInstance() when main thread beginning.
*/

#include <mutex>
#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>

class ThreadPool
{
	typedef std::unique_lock<std::mutex> Lock;
	typedef std::function<void()> Job;

public:
	class JobQueue
	{
	public:
		JobQueue() : _isRunning(true) {}

	public:
		void pushJob(const Job& job)
		{
			Lock lock(_mutex);

			if(!_isRunning)
				return;

			_jobs.push(job);
			_condition.notify_one();
		}

		Job getJob()
		{
			Lock lock(_mutex);

			if(_jobs.empty() && _isRunning)
				_condition.wait(lock);

			if(_jobs.empty())// Wait all of the job finished.
				return Job(nullptr);

			Job job(_jobs.front());
			_jobs.pop();
			return job;
		}

		void joinJob()
		{
			Lock l(_mutex);

			_isRunning = false;
			_condition.notify_all();
		}

		bool isRunning()
		{
			Lock l(_mutex);

			return _isRunning;
		}

	private:
		std::condition_variable _condition;
		std::queue<Job> _jobs;
		std::mutex _mutex;
		volatile bool _isRunning;
	};

public:
	~ThreadPool() 
	{ 
		if(_jobs.isRunning()) {
			for(auto& it : _threads) it.detach(); 
		}
	}

private:
	ThreadPool() { init(); }

public:
	static ThreadPool* getInstance()
	{
		static ThreadPool s_instance;
		return &s_instance;
	}

public:
	void post(const Job& task)
	{
		_jobs.pushJob(task);
	}

	void join()
	{
		if(_jobs.isRunning()) {

			_jobs.joinJob();
			for(auto& it : _threads)
				it.join();
		}
	}

	void attach(const void* obj)
	{
		Lock lock(s_mutex);

		_attachObjs.push_back(const_cast<void*>(obj));
	}

	void detach(const void* obj)
	{
		Lock lock(s_mutex);

		auto itFind = std::find(_attachObjs.begin(), _attachObjs.end(), obj);
		if(_attachObjs.end() != itFind)
			_attachObjs.erase(itFind);
	}

	bool isObjAlive(const void* obj)
	{
		Lock lock(s_mutex);

		auto itFind = std::find(_attachObjs.begin(), _attachObjs.end(), obj);
		if(_attachObjs.end() != itFind)
			return true;

		return false;
	}

protected:
	void init()
	{
		_threads.reserve(s_threadCount);
		for(int i = 0; i < s_threadCount; ++i)
			_threads.push_back(std::thread(std::bind(&ThreadPool::run, this)));
	}

	void run()
	{
		while(1) {
			Job job = _jobs.getJob();
			if(nullptr != job) {
				job();
			} else if(!_jobs.isRunning()) {
				break;
			}
		}
	}

private:
	JobQueue _jobs;
	std::mutex s_mutex;
	std::vector<void*> _attachObjs;
	std::vector<std::thread> _threads;
	static const int s_threadCount = 1;
};

#define THREADMGR ThreadPool::getInstance()

#endif //_ThreadPool_2015_12_2_38_H_