# Coroutine
C++多线程+协程构建异步并发TCP服务器

多线程/多进程设计的高并发服务器，代码逻辑复杂，调试困难，且可能会存在不同线程中拥有同一文件描述符，特别是使用C++开发的多线程程序，其内存管理不易。对比之下利用多线程+协程实现的高并发服务器代码逻辑简单，编写简单，易于维护，很容易实现业务异步，使得CPU利用率极高，并发效率不逊与多线程实现的异步服务器。

项目主体工作如下：

服务端:
  
  1.使用NtyCo协程库(https://github.com/wangbojing/NtyCo/tree/master/core)。
  
  2.按照接入的客户端数量，以及机器配置决定子线程的数量，主线程预先创建多个线程，每个线程各自accept，同时拥有各自的协程调度器。每个子线程每accept一次创建一协程，每个协程内部异步处理客户端的recv, send, 业务处理，主线程负责统计打印服务端recv,处理的数据包的总数。
  
  3.模拟客户端可向服务端发送多种请求，服务端根据客户端请求向客户端返回对应的响应。服务器支持客户端数据的粘包，分包处理，心跳监测，以及定时发送消息。封装线程调用方便线程的管理，完善日志管理系统。
  
模拟客户端:模拟客户端主要的任务是测试服务端的性能。

  1.

 
  