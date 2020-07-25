#strIP="154.8.151.112"
strIP="127.0.0.1"
nPort=4567
nThread=2
nClient=1000
nMsg=1
nSendInterval=0
nSendBuffSize=1024
nRecvBuffSize=1024
checkMsgID=1
./client $strIP $nPort $nThread $nClient $nMsg $nSendInterval $nSendBuffSize $nRecvBuffSize $checkMsgID
