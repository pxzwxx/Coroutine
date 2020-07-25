#ifndef _EasyTc_pClient_hpp_
#define _EasyTc_pClient_hpp_

#include "CELL.hpp"
#include "CELLClient.hpp"
#include "CELLEpoll.hpp"

class EasyTcpClient {
public:
	EasyTcpClient(int nMsg, int nSendInterval, bool bCheckMsgID) : _pClient(nullptr), _isConnect(false), _nMsg(nMsg), _nSendInterval(nSendInterval), _bCheckMsgID(bCheckMsgID) {
		_nSendCount = _nMsg;
	}
	virtual ~EasyTcpClient() {
		Close();
	}
	//InitSocket
	SOCKET InitSocket(int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE) {
		if (_pClient) {
			strerror(errno);
			Log::WARN("<socket=%d> is existed\n", _pClient->sockfd());
			delete _pClient;
			_pClient = nullptr;
			Close();
		}
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			Log::ERROR("socket create error : <socket=%d> is \n", _pClient->sockfd());
		}
		else {
			_pClient = new CELLClient(_sock, sendSize, recvSize);
		}
		//创建epoll管理客户端fd
		_ep.create(1);
		_ep.ctl(EPOLL_CTL_ADD, _pClient, EPOLLIN);
		return _sock;
	}

	//Connect Server
	int Connect(const char* ip,unsigned short port) {
		if (_pClient == nullptr) {
			if (INVALID_SOCKET == InitSocket()) 
				return SOCKET_ERROR;
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		_sin.sin_addr.s_addr = inet_addr(ip);

		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			strerror(errno);
			Log::ERROR("<socket=%d>, connect<%s:%d>error...\n", _pClient->sockfd(), ip, port);
		}
		else {
			_isConnect = true;
		}
		return ret;
	}

	//closesocket
	void Close() {
		if (_pClient){
			close(_pClient->sockfd());
			delete _pClient;
			_pClient = nullptr;
		}
		_ep.destory();
		_isConnect = false;
	}

	//recv, 处理网络消息, send
	bool OnRun(int microseconds = 1) {
		if (isRun()) {
			int ret = -1;
			if (_pClient->needWrite()) {
				_ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN|EPOLLOUT);
			}
			else {
				_ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN);
			}
			ret = _ep.wait(microseconds);
			if (ret < 0) {
				Log::ERROR("EasyTcpClient.OnRun.wait");
				Close();
				return false;
			}
			else if (ret == 0) {
				return true;
			}
			auto events = _ep.events();
			for (int i = 0; i < ret; i ++) {
				CELLClient* pClient = (CELLClient*)events[i].data.ptr;
				if (pClient) {
					if (events[i].events & EPOLLIN) {
						if (SOCKET_ERROR == RecvData()) {
							Log::ERROR("EasyTcpClient.OnRun.RecvData exit \n");
							Close();
							continue;
						}
					}
					if (events[i].events & EPOLLOUT) {
						if (SOCKET_ERROR == pClient->SendDataReal()) {
							Log::ERROR("EasyTcpClient.OnRun.SendDataReal exit \n");
							Close();
							continue;
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//isRun
	bool isRun() {
		return _pClient && _isConnect;
	}

	//RecvData
	int RecvData() {
		int nLen = _pClient->RecvData();
		if (nLen > 0) {
			while (_pClient->hasMsg()) {
				OnNetMsg(_pClient->front_msg());
				_pClient->pop_front_msg();
			}
		}
		return nLen;
	}

	//处理网络消息
	void OnNetMsg(netmsg_DataHeader* header) {
		_nSend = false;
		switch (header->cmd) {
			case CMD_LOGIN_RESULT: {
				netmsg_LoginR* login = (netmsg_LoginR*)header;
				if (_bCheckMsgID) {
					if (login->msgID != _nRecvMsgID) {
						printf("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d, %d>", _pClient->sockfd(),
							login->msgID, _pClient->_nRecvMsgID, login->msgID - _nRecvMsgID);
						Log::WARN("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d, %d>", _pClient->sockfd(),
							login->msgID, _pClient->_nRecvMsgID, login->msgID - _nRecvMsgID);
					}
					++_nRecvMsgID;
				}
			}break;
			case CMD_LOGOUT_RESULT: {
				netmsg_LogoutR* logout = (netmsg_LogoutR*)header;
			}break;
			case CMD_NEW_USER_JOIN: {
				netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
			}break;
			case CMD_ERROR: {
				Log::ERROR("<socket=%d>接收数据出错, dataLength = %d\n", _pClient->sockfd(), header->dataLength);
			}break;
			default:{
				Log::ERROR("<socket=%d>接收未知数据, dataLength = %d\n", _pClient->sockfd(), header->dataLength);
			}
		}
	}

	//按照规则发送数据
	int SendTest(netmsg_Login* login) {
		int ret = 0;
		if (_nSendCount > 0 && _nSend == false) { 
			login->msgID = _nSendMsgID;
			ret = _pClient->SendData2Buffer(login);
			if (SOCKET_ERROR != ret) {
				_nSend = true;  //只有确定收到消息后才能再次发送
				++_nSendMsgID;  //发送计数++
				if(_nSendInterval) { //设置每秒种固定发送多少数据
					--_nSendCount;
				}
			}
			else {
				Close();
			}
		}
		return ret;
	}

	//发送时间检测，每间隔_nSendSleep时间，将发送计算置位
	bool checkSend(time_t dt) {
		_tResetTime += dt;
		if (_tResetTime >= _nSendInterval) {
			_tResetTime -= _nSendInterval;
			_nSendCount = _nMsg; //
		}
		return _nSendCount > 0;
	}

protected:
	CELLClient* _pClient;
	bool _isConnect;
	CELLEpoll _ep;
	
	//接收计数
	int _nRecvMsgID = 1;
	//发送计算
	int _nSendMsgID = 1;
	//在_nSendInterval时间内发送_nMsg条数据
	int _nMsg = 0;

	int _nSendCount = 0;     //在_nSendInterval时间内发送消息统计计数
	time_t _tResetTime = 0;	 //重置时间
	bool _bCheckMsgID;		 //检查消息序号正确否
	int  _nSendInterval = 0; //发送时间间隔
	bool _nSend = false;     //一问一答形式
};

#endif
