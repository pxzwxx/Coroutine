#ifndef _CELL_EPOLL_HPP_
#define _CELL_EPOLL_HPP_

#if __linux__
#include"CELL.hpp"
#include"CELLClient.hpp"

#define EPOLL_ERROR (-1)

class CELLEpoll {
public:
	~CELLEpoll() {
		destory();
	}
	int create(int nMaxEvents) {
		if(_epfd > 0) {
			destory();
		}		

		_epfd = epoll_create(nMaxEvents);
		if(EPOLL_ERROR == _epfd) {
			printf("nMaxEvents = %d\n", nMaxEvents);
			strerror(errno);
			perror("epolll_create");
			return EPOLL_ERROR;	
		}
		if(_pEvents == nullptr) {
			_pEvents = new epoll_event[nMaxEvents];
		}	
		_nMaxEvents = nMaxEvents;
		return 0;
	}
	int ctl(int op, SOCKET cSock, uint32_t events) {
		epoll_event ev;
		ev.events = events;
		ev.data.fd = cSock;
		if(epoll_ctl(_epfd, op, cSock, &ev) == EPOLL_ERROR) {
			perror("error, epoll_ctl");
			return EPOLL_ERROR;
		}
    	return 0;
    }
	
	int ctl(int op, CELLClient* pClient, uint32_t events) {
		epoll_event ev;
		ev.events = events;
		ev.data.ptr = pClient;
		if(epoll_ctl(_epfd, op, pClient->sockfd(), &ev) == EPOLL_ERROR) {
			perror("error, epoll_ctl");
			return EPOLL_ERROR;
		}
    	return 0;
    }

	int wait(int timeout) { //以毫秒为单位
        int ret = epoll_wait(_epfd, _pEvents, _nMaxEvents, timeout);
        if (ret < 0) { //error
            perror("error, epoll_wait");
            return ret;
        }
		return ret;
	}
	epoll_event *events() {
		return _pEvents;
	}
	void destory() {
		if(_epfd > 0) {
			close(_epfd);
			_epfd = -1;
		}
		if(_pEvents) {
			delete[] _pEvents;
			_pEvents = nullptr;
		}
	}
	
private:
	int _epfd = -1;
	int _nMaxEvents = 0;
	epoll_event * _pEvents = nullptr;
};

#endif
#endif
