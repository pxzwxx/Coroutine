#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include"../share/AsyBuffer.hpp"
#include"CELL.hpp"

//客户端数据类型
class CELLClient
{
public:
	//客户端id
	int id = -1;
	//所属serverid
	int serverId = -1;
	bool _needWrite;
public:
	CELLClient(SOCKET sockfd = INVALID_SOCKET, int sendBuffSize = SEND_BUFF_SZIE, int recvBuffSize = RECV_BUFF_SZIE) :_recvBuff(recvBuffSize), _sendBuff(sendBuffSize)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
		resetDTSend();
		_expire = time(NULL) + 1 * TIMESLOT;
	}

	~CELLClient()
	{
		Log::INFO("s=%d CELLClient%d.~CELLClient\n", serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
			close(_sockfd);
			_sockfd = INVALID_SOCKET;
		}
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	//协程异步读socket
	int RecvData()
	{
		int ret = _recvBuff.read4socket(_sockfd);
		if(ret > 0) { //心跳时间更新
			_expire = time(NULL) + 1 * TIMESLOT;
		}
		return ret;
	}

	//判断是否接收到完整的消息
	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	//获取第一条消息
	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuff.data();
	}

	void pop_front_msg()
	{
		if(hasMsg())
			_recvBuff.pop(front_msg()->dataLength);
	}

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		resetDTSend();
		int ret = _sendBuff.write2socket(_sockfd);
		if(ret > 0) {
			_expire = time(NULL) + 1 * TIMESLOT;
		}
		return ret;
	}

	//发送数据到发送缓冲区
	int SendData2Buffer(netmsg_DataHeader* header)
	{
		if (_sendBuff.push((const char*)header, header->dataLength))
		{
			return header->dataLength;
		}
		return SOCKET_ERROR;
	}
	
	//预留定时发送接口
	void resetDTSend()
	{
		_dtSend = 0;
	}

	//获取心跳时间
	time_t getExpired() {
		return _expire;
	}

	//支持定时发送消息检测，本次代码没有使用该功能
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			_needWrite = true;
			resetDTSend();
			return true;
		}
		return false;
	}

	//判断是否需要写socket
	bool needWrite() {
		return _sendBuff.needWrite();
	}

private:
	SOCKET _sockfd;
	//接收消息缓冲区
	AsyBuffer _recvBuff;
	//发送缓冲区
	AsyBuffer _sendBuff;
	//上次发送消息数据的时间 
	time_t _dtSend;
	//给个定时器, 记录客户端的心跳时间
	time_t _expire;	
public:
	//以下三标志，是为了判断服务端是否正确处理客户端发来的请求，只是为了测试服务端业务处理的正确性
	int _nRecvMsgID = 1;  //接受消息id
	int _nSendMsgID = 1;  //发送消息id
	int _nSendCount = 0;  //发送计数
};

#endif // !_CELLClient_HPP_
