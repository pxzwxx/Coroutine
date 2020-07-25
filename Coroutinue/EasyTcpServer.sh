#strIP="154.8.151.112"
strIP="any"  #any代表绑定任意本地ip，当前也可以设置自己的ip
nPort=4567   #设置绑定的端口
nThread=4    #启动的子服务数
nClient=2000 #当前服务器，允许接入的最大客户端数
./server $strIP $nPort $nThread $nClient
