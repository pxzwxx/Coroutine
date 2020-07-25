#include"log.h"

std::mutex Log::m_mutex;
Log *Log::m_log = NULL;

semphore Log::m_fullBuff;
semphore Log::m_emptyBuff;
std::list<std::string> Log::m_list;

//创建日志单例类
Log* Log::GetLogInstance() {
    if(m_log == NULL) {
        std::unique_lock<std::mutex> ulock(m_mutex);
        if(m_log == NULL) {
            m_log = new Log;
            if(m_log == NULL) return NULL;
        }
    }
    return m_log;
}

//从缓冲队列取日志消息
void Log::async_write_log() {
    while(isRun) {
        m_fullBuff.P();
        m_mutex.lock();
        auto item = m_list.front();
        m_list.pop_front();

        fputs(item.c_str(), m_fp);
        fflush(m_fp);

        m_mutex.unlock();
        m_emptyBuff.V();
    }
}

//初始化日志名，最大并发量
void Log::init(const char *file_name, int max_queue_size) {
    m_max_task = max_queue_size;
    memcpy(m_log_name, file_name, strlen(file_name) + 1);
    m_fp = fopen(file_name, "a");  //追加模式日志文件   
    m_fullBuff.setNum(0);           //设置信号量值
    m_emptyBuff.setNum(m_max_task); //设置信号量值
    log_thread = std::thread(&Log::async_write_log, this);
    log_thread.detach();
}

void Log::destory() {
    isRun = false;
    if(m_log) {
        std::unique_lock<std::mutex> ulock(m_mutex);
        if(m_fp) {
            fflush(m_fp);
            fclose(m_fp);
        }
        if(m_log) delete m_log;
        m_max_task = false;
        m_log = NULL;
        m_fp = NULL;
    }
}

