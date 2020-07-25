#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include"lock.h"
#include<thread>
#include<functional>

/*
	封装线程类 : 可以根据业务的需求，任意的开启，关闭，和退出线程，并使用C++11提供的包装器，轻松的实现业务和代码的分离
	notes : C++11的多线程目前没有提供信号量，为了实现线程的安全退出过程，借用C++线程的条件变量实现了一个信号量
*/

class CELLThread
{
public:
	static void Sleep(time_t dt) {
		std::chrono::milliseconds t(dt);
		std::this_thread::sleep_for(t);
	}
private:
	typedef std::function<void(CELLThread*)> EventCall;
public:
	//启动线程
	void Start(
		EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)
				_onCreate = onCreate;
			if (onRun)
				_onRun = onRun;
			if (onDestory)
				_onDestory = onDestory;

			//启动线程
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);
			t.detach();
		}
	}

	//关闭线程
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			m_sem.P();
		}
	}
	//在工作函数中退出
	//不需要使用信号量来阻塞等待
	//如果使用会阻塞
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}

	//线程是否启动运行状态
	bool isRun()
	{
		return _isRun;
	}
protected:
	//线程的运行时的工作函数
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		m_sem.V();
	}
private:
	EventCall _onCreate; //线程启动前工作
	EventCall _onRun;    //线程工作函数
	EventCall _onDestory;//线程结束的善后函数
	//不同线程中改变数据时需要加锁
	std::mutex _mutex;
	//控制线程的终止、退出
	semphore m_sem;
	//线程是否启动运行中
	bool	_isRun = false;
};


#endif // !_CELL_THREAD_HPP_
