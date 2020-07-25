#ifndef _ASYBUFFER_HPP_
#define _ASYBUFFER_HPP_

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define RECV_BUFF_SZIE 1024 * 5     //接受数据缓冲区size
#define SEND_BUFF_SZIE 1024 * 5     //发送数据缓冲区size
#define TIMESLOT 60 * 2             //心跳活动时间2分钟
#define CLIENT_SEND_BUFF_TIME 200   //定时发送时间

#include"MessageHeader.hpp"
#include"log.h"
extern "C" {
    #include"../ntyco/nty_coroutine.h"
}

/*
	封装读写缓冲区：包含一个缓冲Buff, 支持异步协程recv, send套接口，以及处理分包，粘包
*/

class AsyBuffer
{
public:
	AsyBuffer(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	~AsyBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data()
	{
		return _pBuff;
	} 

	//向写缓冲区写入一条完整的消息
	bool push(const char* pData, int nLen)
	{
		if (_nLast + nLen <= _nSize)
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			//计算数据尾部位置
			_nLast += nLen;

			if (_nLast == SEND_BUFF_SZIE)
			{
				++_fullCount;//发送缓冲区已满
				Log::WARN("send buff is full \n");
			}

			return true;
		}
		else {
			++_fullCount;
		}

		return false;
	}
	
	//弹出读缓冲区的第一条完整的消息
	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}
	
	//协程写
	int write2socket(SOCKET sockfd) {
		int ret = 0;
		if (_nLast > 0 && INVALID_SOCKET != sockfd) { //缓冲区有数据
			ret = nty_send(sockfd, _pBuff, _nLast, 0);
			if (ret == -1) {
				return SOCKET_ERROR;
			}
			if (ret == _nLast) {
				_nLast = 0; //数据尾部位置清零
			}
			else {
				_nLast -= ret;
				memcpy(_pBuff, _pBuff + ret, _nLast);
			}
			_fullCount = 0;
		}
		return ret;
	}

	//协程读
	int read4socket(SOCKET sockfd) {
		if (_nSize - _nLast > 0) {
			//接收客户端数据
			char* szRecv = _pBuff + _nLast;
			int nLen = nty_recv(sockfd, szRecv, _nSize - _nLast, 0);
			if (nLen == -1) {
				return SOCKET_ERROR;
			}
			//消息缓冲区的数据尾部位置后移
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	//判断客户端是否接受到一条完整的消息
	bool hasMsg()
	{
		//判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			//这时就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			//判断消息缓冲区的数据长度大于消息长度
			return _nLast >= header->dataLength;
		}
		return false;
	}

	//判断客户端是否可写
	bool needWrite()
	{
		return _nLast > 0;
	}
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//可以用链表或队列来管理缓冲数据块
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
};

#endif // !_CELL_BUFFER_HPP_
