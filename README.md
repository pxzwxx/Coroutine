# Coroutine
C++多线程+协程构建异步并发BS架构服务器


多线程/多进程设计的高并发服务器，代码逻辑复杂，调试困难，且可能会存在不同线程中拥有同一文件描述符，特别是使用C++开发的多线程程序，其内存管理不易。对比之下利用多线程+协程实现的高并发服务器代码逻辑简单，编写简单，易于维护，很容易实现业务异步，CPU利用率极高，并发效率不逊与多线程实现的异步服务器。初学者建议先了解一种经典的使用线程池实现并发服务器https://github.com/pxzwxx/SynBS
从而更好的理解协程的优点。该项目中单线程中所有客户端按照recv, 业务处理，send的顺序同步执行，在业务处理比较单一的情况下执行效率尚可，但若业务中设计到大量IO请求时不可避免出现频繁等待的情况，这种情况下使用协程可大大提高并发网络的效率。

项目主体工作如下：

服务端:
  
  1.使用NtyCo协程库，详见(https://github.com/wangbojing/NtyCo)。
  
  2.按照接入的客户端数量，以及机器配置决定子线程的数量。主线程预先创建多个线程，每个线程各自accept，并拥有各自的协程调度器。每个子线程每accept一次就创建一协程，每个协程代表一个客户端的逻辑，该协程内异步recv, send, 业务处理。主线程负责统计和打印开启的线程数，客户端接入的数量，服务端recv以及处理的数据包总数。
  
  3.模拟客户端可向服务端发送多种请求，服务端根据客户端的请求向客户端返回对应的响应。服务端支持客户端数据的粘包，分包处理，心跳监测，定时发送，以及数据包的丢包，混包监测等功能。
  
  4.封装接受和发送消息缓冲区，缓冲区支持使用协程异步读写socket。
  
  5.封装线程类，方便线程的创建，运行，退出，同时支持异步写日志功能。
  
模拟客户端 : 模拟客户端主要的任务是测试服务端的性能。

  1.模拟客户端多个线程，每个线程内创建多个客户端，并连接服务器，支持以一定的频率向服务端发送消息，接受数据；支持数据包的丢包，混包监测，主线程统计并打印统计信息。
  
  2.使用epoll模型管理每个客户端socket的读写。
  
快速运行:

  1.测试环境:
    Ubuntu版本18.04　　gcc version 7.5.0
    
    notes1:如果gcc版本较低，可能会缺少GNU的库文件，请自行下载安装。
    
    notes2:运行客户端程序时，先检查用户允许打开的文件描述符数，允许打开的文件描述符数要大于客户端数
    
    notes3:运行客户端程序时，需要检查机器的可用的端口范围，以确定可开启的客户端数量。
          
    
  2.编译代码
    
    编译服务端代码:
    
    make server
    
    编译客户端代码:
    
    make client
    
  3.运行代码
    
    3.1 运行服务端代码:
    
      修改EasyTcpServer.sh中的ip，端口等信息,然后运行如下脚本
    
      sh EasyTcpServer.sh
     
    3.2 运行客户端代码:
     
      修改EasyTcpClient.sh中的ip，端口等信息,然后运行如下脚本
    
      sh EasyTcpClient.sh
   
   4.局域网内测试结果
   
   服务端开启6线程，接入客户端6万个，每秒中调用26万次recv函数，处理了26万个数据包，并回应26万个数据包给6万个客户，每个数据包包含100bytes的数据, 并开启了消息丢包，混包监测，1min心跳时间，稳定运行10h，没有出现丢包，混包，以及客户端断开的情况,具体统计信息如下。在业务比较的单一的情况，对比项目https://github.com/pxzwxx/SynBS
   可以发现网络的处理能力优于利用线程池实现的并发服务器，相信在业务中设计较多的IO时操作时，协程的优势会更加明显。
    
   ![image](https://github.com/pxzwxx/Coroutine/blob/master/Coroutinue/server.png)
   
   
   
  
  

 
  
