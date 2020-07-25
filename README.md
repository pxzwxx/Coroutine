# Coroutine
C++多线程+协程构建异步并发BS架构服务器

多线程/多进程设计的高并发服务器，代码逻辑复杂，调试困难，且可能会存在不同线程中拥有同一文件描述符，特别是使用C++开发的多线程程序，其内存管理不易。对比之下利用多线程+协程实现的高并发服务器代码逻辑简单，编写简单，易于维护，很容易实现业务异步，CPU利用率极高，并发效率不逊与多线程实现的异步服务器。

项目主体工作如下：

服务端:
  
  1使用NtyCo协程库，详见(https://github.com/wangbojing/NtyCo)。
  
  2.按照接入的客户端数量，以及机器配置决定子线程的数量。主线程预先创建多个线程，每个线程各自accept，并拥有各自的协程调度器。每个子线程每accept一次就创建一协程，每个协程代表一个客户端的逻辑，该协程内异步recv, send, 业务处理。主线程负责统计和打印开启的线程数，客户端接入的数量，服务端recv以及处理的数据包总数。
  
  3.模拟客户端可向服务端发送多种请求，服务端根据客户端的请求向客户端返回对应的响应。服务端支持客户端数据的粘包，分包处理，心跳监测，定时发送，以及数据包的丢包，混包监测等功能。
  
  4.封装接受和发送消息缓冲区，缓冲区支持使用协程异步读写socket。
  
  ５.封装线程类，方便线程的创建，运行，退出，同时支持异步写日志功能。
  
模拟客户端 : 模拟客户端主要的任务是测试服务端的性能。

  1.模拟客户端多个线程，每个线程内创建多个客户端，并连接服务器，支持以一定的频率向服务端发送消息，接受数据；支持数据包的丢包，混包监测，主线程统计并打印统计信息。
  
  2.使用epoll模型管理每个客户端socket的读写。
  
快速运行:

  1.测试环境:
    Ubuntu版本18.04　　gcc version 7.5.0
    
    notes:如果gcc版本较低，可能会缺少GNU的库文件，请自行下载安装。
    
  2.编译代码
    
    make server
    
    make client
    
  3.运行代码
    
    3.1 运行服务端代码:
    
      修改EasyTcpServer.sh中的ip，端口等信息,然后运行如下脚本
    
      sh EasyTcpServer.sh
     
     3.2 运行客户端代码:
     
      修改EasyTcpClient.sh中的ip，端口等信息,然后运行如下脚本
    
      sh EasyTcpClient.sh
   
   4.局域网内测试结果
   
    ![服务端]https://github.com/pxzwxx/Coroutine/blob/master/server.png
  
  

 
  
