#ifndef _LOCKER_
#define _LOCKER_

#include<iostream>
#include<mutex>
#include<condition_variable>

/*
    使用C++线程封装了一个好用的信号量，支持P,V操作
*/

class semphore {
public:
    semphore(int num = 0) : m_num(num) {}
    ~semphore() {
        m_num = 0;
    }
    void P() {
        std::unique_lock<std::mutex> ulock(m_mutex);
        m_num --;
        if(m_num < 0) m_cv.wait(ulock);
    }
    void V() {
        std::unique_lock<std::mutex> ulock(m_mutex);
        m_num ++;
        if(m_num <= 0) m_cv.notify_one();
    }
    int size() {
        std::unique_lock<std::mutex> ulock(m_mutex);
        return m_num > 0 ? m_num : 0;
    }
	void setNum(int num) {
		m_num = num;
	}
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_num;
};

class locker
{
public:
    locker() {}
    ~locker() {}
    void lock() {
		m_mutex.lock();
    }
    void unlock() {
		m_mutex.unlock();
    }
    std::mutex *get() {
		return &m_mutex;
    }
private:
   std::mutex m_mutex;
};



#endif

