#ifndef _CELL_HPP_
#define _CELL_HPP_


#include<unistd.h> 
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<errno.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#include"../share/MessageHeader.hpp"
#include"../share/CELLTimestamp.hpp"
#include"../share/CELLThread.hpp"
#include"../share/log.h"
#include"../share/lock.h"
#include"../share/SynBuffer.hpp"

#include<stdio.h>
#include<thread>
#include<string>
#include<vector>
#include<atomic>
#include<iostream>

#endif // !_CELL_HPP_
