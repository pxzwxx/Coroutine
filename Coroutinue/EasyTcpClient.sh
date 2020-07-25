#strIP="154.8.151.112"
strIP="127.0.0.1"   #绑定连接ip
nPort=4567          #绑定连接端口
nThread=2           #开启线程数
nClient=1000        #每个线程创建nClient个客户端并连接服务器
nMsg=1              
nSendInterval=0     #0:每个客户端不停的向服务端发送消息，>0:每个客户端在nSendInterval时间内向客户端发送nMsg条消息，主要是为了做压力测试，均衡测试
nSendBuffSize=1024  #设置发送缓冲区大小
nRecvBuffSize=1024  #设置接受缓冲区大小
checkMsgID=1        #bool值，true : 开启客户端和服务端通信丢包，混包监测，false : 关闭此功能
./client $strIP $nPort $nThread $nClient $nMsg $nSendInterval $nSendBuffSize $nRecvBuffSize $checkMsgID
