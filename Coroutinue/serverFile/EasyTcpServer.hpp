#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"CELL.hpp"
#include"CELLServer.hpp"

class EasyTcpServer : public INetEvent {
private:
	//存储子服务
	std::vector<CELLServer*> _cellServers;
	//socket fd
	SOCKET _sock;
	//打印信息时间戳
	time_t _oldCur;
protected:
	//客户端最大连接上限
	int _nMaxClient;	
	//SOCKET recv计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	EasyTcpServer() : _sock(INVALID_SOCKET), _recvCount(0),_msgCount(1), _clientCount(0), _nMaxClient(0) {
		_oldCur = time(NULL);
	}
	void setMaxClient(int maxClient) {
		_nMaxClient = maxClient; 
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//初始化Socket
	SOCKET InitSocket() {
		if (INVALID_SOCKET != _sock) {
			Log::WARN("warning, initSocket close old socket<%d>...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			Log::ERROR("error, create socket failed...\n");
		}
		else {
			//设置文件描述符号为非阻塞
			int ret = fcntl(_sock, F_SETFL, O_NONBLOCK); 
			if (ret == -1) {
				close(ret);
				return -1;
			}
			//设置端口复用
			int flag = 1;
			if(SOCKET_ERROR == setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int))) {
				Log::ERROR("setsockopt socket<%d> SO_REUSEADDR fail",(int)(_sock));
				return SOCKET_ERROR;
			}
			Log::INFO("create socket<%d> success...\n", (int)_sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port) {
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			Log::ERROR("error, bind port<%d> failed...\n", port);
		}
		else {
			Log::INFO("bind port<%d> success...\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			Log::ERROR("error, listen socket<%d> failed...\n",_sock);
		}
		else {
			Log::INFO("listen port<%d> success...\n", _sock);
		}
		return ret;
	}
	
	//启动nCELLServer个子服务
	void Start(int nCELLServer) {
		for (int n = 0; n < nCELLServer; n++) {
			auto ser = new CELLServer;
			ser->setId(n+1);

			ser->setFd(_sock);  //设置accept描述符号
			ser->setMaxClient(int(_nMaxClient / nCELLServer)); //设置每个服务的可以接受的最大客户端数量

			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
	}

	//关闭Socket
	void Close()
	{
		Log::INFO("EasyTcpServer.Close begin\n");
		if (_sock != INVALID_SOCKET) {
			for (auto s : _cellServers) {
				delete s;
			}
			_cellServers.clear();
			close(_sock);
			_sock = INVALID_SOCKET;
		}
		Log::INFO("EasyTcpServer.Close end\n");
	}
	
	//虚函数重写	
	virtual void OnNetJoin(CELLClient* pClient) {
		_clientCount++;
		Log::INFO("client<%d> OnNetjoin\n", pClient->sockfd());
	}
	
	//虚函数重写
	virtual void OnNetLeave(CELLClient* pClient) {
		_clientCount--;
		Log::INFO("client<%d> OnNetleave\n", pClient->sockfd());
	}

	//虚函数重写
	virtual void OnNetRecv(CELLClient* pClient) {
		_recvCount++;
	}
	
	//虚函数重写
    virtual void OnNetMsg(CELLClient* pClient) {
		_msgCount++;
    }

	//打印统计信息
	void OnRun() {
		while (true) {
			time4msg();
		}
	}

protected:
	SOCKET sockfd() {
		return _sock;
	}
	//计算并输出每秒收到的网络消息
	void time4msg() {
		time_t cur = time(NULL);
		int t = (int)(cur - _oldCur);
		if (t >= 1) {
			printf("thread<%d>,time<%d>,socket<%d>,clients<%d>,recv<%d>,msg<%d>\n", (int)_cellServers.size(), t, _sock, (int)_clientCount, (int)(_recvCount), (int)(_msgCount));
			Log::INFO("thread<%d>,time<%d>,socket<%d>,clients<%d>,recv<%d>,msg<%d>\n", (int)_cellServers.size(), t, _sock, (int)_clientCount, (int)(_recvCount), (int)(_msgCount));
			_recvCount = 0;
			_msgCount = 0;
			_oldCur = cur;
		}
	}
};

#endif // !_EasyTcpServer_hpp_
