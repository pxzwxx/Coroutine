#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"CELL.hpp"
#include"INetEvent.hpp"
#include"CELLClient.hpp"
#include<vector>
#include<map>
#include<sstream>
#include<mutex>

/*
	子服务类：每个子服务开启一个线程，每个线程各自accept	
*/

class CELLServer {
public:
	CELLServer() {
		_pNetEvent = nullptr;
	}

	virtual	~CELLServer() {
		Log::INFO("CELLServer%d.~CELLServer exit begin\n", _id);
		Close();
		Log::INFO("CELLServer%d.~CELLServer exit end\n", _id);
	}
	
	//设置当前服务id
	void setId(int id) {
		_id = id;
	}

	//传入accept的socket fd
	void setFd(int fd) {
		_accfd = fd;
	}

	//设置每个线程内接入的客户端上限
	void setMaxClient(int maxClient) {
		_maxClient = maxClient;
		_num = _maxClient;
	}

	
	void setEventObj(INetEvent* event) {
		_pNetEvent = event;
	}

	//关闭Socket
	void Close() {
		Log::INFO("CELLServer%d.Close begin\n", _id);
		_thread.Close();
		Log::INFO("CELLServer%d.Close end\n", _id);
	}

	/**********************************协程实现****************************************/	
	//recv, send, 网络处理协程
	static void NetEvent(void* arg) {
        //参数解包，这里我也不想如此麻烦的解包参数，因为ntyco库里协程执行函数只支持传递一个参数
        int len = sizeof(void *); //计算当前cpu的指针大小
        char *ptr = (char*)arg;
        long long arg1;
        memcpy(&arg1, ptr, len);
        ptr += len;
        long long arg2;
        memcpy(&arg2, ptr, len);
        ptr += len;
        long long arg3;
        memcpy(&arg3, ptr, len);

		CELLServer *server  = (CELLServer *)arg1;
		CELLThread *pThread = (CELLThread *)arg2;
		CELLClient *pClient = (CELLClient *)arg3; 
		while(pThread->isRun()) {

			//协程recv socket
			int ret = pClient->RecvData();
			if(SOCKET_ERROR == ret) {
				server->rmClient(pClient); 
				return; //让出cpu
			}
			else if(ret > 0) { //接收消息成功, 接收计数++
				server->_pNetEvent->OnNetRecv(pClient);//这里不需要加锁，外部已经设置了原子操作了
			}
			
			//处理网络消息			
			while (pClient->hasMsg()) {	
				netmsg_DataHeader* header = pClient->front_msg();	
				switch (header->cmd) {
					case CMD_LOGIN: {
						netmsg_Login* login = (netmsg_Login*)header;
						if (server->_bCheckMsgID) {
							if (login->msgID != pClient->_nRecvMsgID) {
								Log::ERROR("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d>",pClient->sockfd(),login->msgID,pClient->_nRecvMsgID);
							}
							++pClient->_nRecvMsgID;
						}
						if (server->_bSendBack) {
							netmsg_LoginR ret;
							ret.msgID = pClient->_nSendMsgID;
							if (SOCKET_ERROR == pClient->SendData2Buffer(&ret)) {
								if (server->_bSendFull) {
									Log::ERROR("<socket=%d> Send Full", pClient->sockfd());
								}
							}
							else {
								++pClient->_nSendMsgID;
							}
						}
					}break;
					default: {
						Log::ERROR("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
					}break;
        		}				
				pClient->pop_front_msg(); //移除消息队列（缓冲区）最前的一条数据
				server->_pNetEvent->OnNetMsg(pClient);//添加消息计数 
			}
			
			//协程send socket
			if(SOCKET_ERROR == pClient->SendDataReal()) {
				server->rmClient(pClient);
				return;
			}

			//当前客户端心跳监测，超时间剔除
			time_t cur = time(NULL);
			if(cur >= pClient->getExpired()) {
				server->rmClient(pClient);
				return; 
			}
		}
	}	
	
	//accpet协程，当前协程只复杂accept请求
	static void server(void* arg) {
		//参数解包
		int len = sizeof(void *); //计算当前cpu的指针大小
        char *ptr = (char*)arg;
        long long arg1;
        memcpy(&arg1, ptr, len);
        ptr += len;
        long long arg2;
        memcpy(&arg2, ptr, len);
        ptr += len;

		CELLServer *server  = (CELLServer *)arg1;
		CELLThread *pThread = (CELLThread *)arg2;
		while (pThread->isRun()) {
			struct sockaddr_in remote;
			socklen_t len = sizeof(struct sockaddr_in);
			
			while(server->_num == 0) {//当前线程的连接客户端数量达到最大时，睡眠当前协程，让出cpu
				nty_coroutine_sleep(100); //睡眠100毫秒
			}

			int cli_fd = nty_accept(server->_accfd, (struct sockaddr*)&remote, &len); //新的客户端连接到来

			server->_num --;
			CELLClient *pClient = new CELLClient(cli_fd);
			pClient->serverId = server->_id;

			if (server->_pNetEvent)
				server->_pNetEvent->OnNetJoin(pClient);

			//参数打包
			char arg[64] = {0};
			char *ptr = arg;
			int plen = sizeof(void *); //求当前CPU的地址位数

			long long src =  (long long)server;
			memcpy(ptr, (const void*)&src, plen);
			ptr += plen;
			
			src =  (long long)pThread;
			memcpy(ptr, (const void*)&src, plen);
			ptr += plen;
		
			src =  (long long)pClient;
			memcpy(ptr, (const void*)&src, plen);
			
			nty_coroutine *co;
			nty_coroutine_create(&co, NetEvent, (void*)arg);
		}
		nty_coroutine *co = nty_coroutine_get_sched()->curr_thread;
		nty_coroutine_yield(co);//让出cpu
	}

	//线程运行函数
	void OnRun(CELLThread* pThread) {
		//参数打包		
		char arg[64] = {0};
		char *ptr = arg;
		int len = sizeof(void *); //求当前CPU的地址位数
 		long long src = (long long)this;
    	memcpy(ptr, (const void*)&src, len);
    	ptr += len;

 		src =  (long long)pThread;
    	memcpy(ptr, (const void*)&src, len);

		nty_coroutine *co;
		nty_coroutine_create(&co, server, arg);	
		nty_schedule_run(); //协程调度	
		printf("调度器退出\n");	
	}

	/*********************************协程实现***************************************/

	//开启服务线程
	void Start() {
		_thread.Start(
			nullptr,
			[this](CELLThread* pThread) {
				OnRun(pThread);
			},
			nullptr
		);
	}

	//删除当前客户连接
	void rmClient(CELLClient *pClient) {
		printf("客户端退出 = %p\n", pClient);
		Log::INFO("客户端超时pClient = 　%p\n", pClient);
		close(pClient->sockfd()); //关闭文件描述符
		_pNetEvent->OnNetLeave(pClient);
		delete pClient;
		pClient = NULL;

		this->_num ++; //当前线程可接入的客户数量+1
		nty_coroutine *co = nty_coroutine_get_sched()->curr_thread;
		nty_coroutine_yield(co);//让出cpu
	}
	
protected:
	//客户列表是否有变化
	int _id = -1;
	//网络事件对象
	INetEvent* _pNetEvent;
private:
	//任务线程句柄
	CELLThread _thread;
	//消息检查标志位
	bool _bCheckMsgID = true;
    bool _bSendBack = true;
    bool _bSendFull = true;
	int  _accfd = -1;
	int  _maxClient = 0;
	int  _num; //_num线程的负载均衡计数，保证每个线程接受连接的数据大致相同
	static std::mutex _mutex;
};

#endif // !_CELL_SERVER_HPP_

