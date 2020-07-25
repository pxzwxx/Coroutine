#ifndef _CELL_HPP_
#define _CELL_HPP_

#include<unistd.h> 
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>
#include<sys/socket.h>
#include<errno.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#include"../share/MessageHeader.hpp"
#include"../share/CELLThread.hpp"
#include"../share/lock.h"
#include"../share/log.h"
#include"../share/INetEvent.hpp"

#include<string>
#include<stdio.h>
#include<thread>
#include<mutex>
#include<atomic>
#include<iostream>
#include<vector>

extern "C" {
    #include"../ntyco/nty_coroutine.h"
}

#endif // !_CELL_HPP_

