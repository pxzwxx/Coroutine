#ifndef _LOG_
#define _LOG_
#include"lock.h"
#include<cstring>
#include<mutex>
#include<list>
#include<thread>
#include<time.h>
#include<stdio.h>


//异步写日志，即先把日志消息写入消息缓冲区，然后从缓冲区取写入到文件
class Log {
private:
	Log() : isRun(true) {}
	Log(Log &tmp) = delete;
	Log & operator=(Log &tmp) = delete;	
public:
	~Log() {
		if(m_fp) destory();	
	}
public:
	static Log* GetLogInstance();

	template<typename ...Args>	
	static void DEBUG(const char *format, Args ...args) {
        m_emptyBuff.P();
        m_mutex.lock();
      
        char log_buf[10240] = {0};
        time_t t = time(NULL);
        struct tm *sys_tm = localtime(&t);
        sprintf(log_buf, "%d-%d-%d-%d-%d", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, sys_tm->tm_hour, sys_tm->tm_min);
        int n = strlen(log_buf);
        sprintf(log_buf + n, "  %s", "[debug] : ");
        n = strlen(log_buf);
        sprintf(log_buf + n, format, args...);
        std::string s(log_buf);

        m_list.push_back(std::move(s));

        m_mutex.unlock();
        m_fullBuff.V();
	}	

	template<typename ...Args>	
	static void INFO(const char *format, Args ...args) {
        m_emptyBuff.P();
        m_mutex.lock();
      
        char log_buf[10240] = {0};
        time_t t = time(NULL);
        struct tm *sys_tm = localtime(&t);
        sprintf(log_buf, "%d-%d-%d-%d-%d", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, sys_tm->tm_hour, sys_tm->tm_min);
        int n = strlen(log_buf);
        sprintf(log_buf + n, "  %s", "[info]  : ");
        n = strlen(log_buf);
        sprintf(log_buf + n, format, args...);
        std::string s(log_buf);

        m_list.push_back(std::move(s));

        m_mutex.unlock();
        m_fullBuff.V();
	}	

	template<typename ...Args>	
	static void WARN(const char *format, Args ...args) {
        m_emptyBuff.P();
        m_mutex.lock();
      
        char log_buf[10240] = {0};
        time_t t = time(NULL);
        struct tm *sys_tm = localtime(&t);
        sprintf(log_buf, "%d-%d-%d-%d-%d", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, sys_tm->tm_hour, sys_tm->tm_min);
        int n = strlen(log_buf);
        sprintf(log_buf + n, "  %s", "[warn]  : ");
        n = strlen(log_buf);
        sprintf(log_buf + n, format, args...);
        std::string s(log_buf);

        m_list.push_back(std::move(s));

        m_mutex.unlock();
        m_fullBuff.V();
	}	

	template<typename ...Args>	
	static void ERROR(const char *format, Args ...args) {
        m_emptyBuff.P();
        m_mutex.lock();
      
        char log_buf[10240] = {0};
        time_t t = time(NULL);
        struct tm *sys_tm = localtime(&t);
        sprintf(log_buf, "%d-%d-%d-%d-%d", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, sys_tm->tm_hour, sys_tm->tm_min);
        int n = strlen(log_buf);
        sprintf(log_buf + n, "  %s", "[error] : ");
        n = strlen(log_buf);
        sprintf(log_buf + n, format, args...);
        std::string s(log_buf);

        m_list.push_back(std::move(s));

        m_mutex.unlock();
        m_fullBuff.V();
	}	
        void init(const char *file_name, int max_queue_size);
        
private:
	//从缓冲队列取日志消息
	void async_write_log();
	void destory();
//public:
	static semphore m_fullBuff, m_emptyBuff;
	int m_max_task;
	FILE *m_fp;
	char m_log_name[128];
	static std::mutex m_mutex;
	static Log *m_log;
	static std::list<std::string> m_list;
	std::thread log_thread;
	bool isRun;
};


#endif

