#include "EasyTcpClient.hpp"
#include "CELL.hpp"

const char* strIP = nullptr;
uint16_t nPort = 0;
int  nThread = 0;
int  nClient = 0;

int  nSendBuffSize = SEND_BUFF_SZIE;
int  nRecvBuffSize = RECV_BUFF_SZIE;
int  nWorkSleep = 1;

int  nMsg = 1;
int  nSendInterval = 0; //0 : 客户端不停的发送，　其它值：每个客户端nSendInterval间隔内发送nMsg条数据
bool checkMsgID = false;

//计数统计变量
std::atomic<int> sendCount(0);
std::atomic<int> readyCount(0);
std::atomic<int> nConnect(0);


void WorkThread(CELLThread* pThread, int id) {
	std::vector<EasyTcpClient*> clients(nClient);
	int begin = 0, end = nClient;
	for (int n = begin; n < end; n ++) {
		if (!pThread->isRun())
			break;
		clients[n] = new EasyTcpClient(nMsg, nSendInterval, checkMsgID);
		CELLThread::Sleep(0);
	}
	for (int n = begin; n < end; n ++) {
		if (!pThread->isRun())
			break;
		if (INVALID_SOCKET == clients[n]->InitSocket(nSendBuffSize, nRecvBuffSize))
			break;
		if (SOCKET_ERROR == clients[n]->Connect(strIP, nPort))
			break;
		nConnect++;
		CELLThread::Sleep(0);
	}

	readyCount++;
	while (readyCount < nThread && pThread->isRun()) {
		CELLThread::Sleep(10);
	}

	netmsg_Login login;
	strcpy(login.userName, "abc");
	strcpy(login.PassWord, "xyz");

	auto t2 = CELLTime::getNowInMilliSec();
	auto t0 = t2;
	auto dt = t0;
	CELLTimestamp tTime;
	while (pThread->isRun()) {
		t0 = CELLTime::getNowInMilliSec();
		dt = t0 - t2;
		t2 = t0;
		{
			for (int m = 0; m < nMsg; m ++) {
				for (int n = begin; n < end; n ++) {
					if (clients[n]->isRun()) {
						if (clients[n]->SendTest(&login) > 0) {
							++sendCount;
						}
					}
				}
			}
			for (int n = begin; n < end; n ++) {
				if (clients[n]->isRun()) {
					if (!clients[n]->OnRun(0)) {
						nConnect--;
						continue;
					}
					if(nSendInterval) {
						clients[n]->checkSend(dt);
					}
				}
			}
		}
		CELLThread::Sleep(nWorkSleep);
	}

	for (int n = begin; n < end; n ++) {
		clients[n]->Close();
		delete clients[n];
	}
	--readyCount;
}

int main(int argc, char *args[]) {

	Log::GetLogInstance()->init("./clientLog",100000);
	Log::INFO("--------------------server is start--------------------\n");

	if (argc < 9) Log::ERROR("main parameters error : argc = %d\n", argc);
	
	strIP = args[1];
	nPort = (uint16_t)std::stoi(args[2]);
	nThread = std::stoi(args[3]);
	nClient = std::stoi(args[4]);
	nMsg = std::stoi(args[5]);
	nSendInterval = std::stoi(args[6]);
	nSendBuffSize = std::stoi(args[7]);
	nRecvBuffSize = std::stoi(args[8]);
	checkMsgID = (bool)std::stoi(args[9]);

	//创建线程
	std::vector<CELLThread*> threads;
	for (int n = 0; n < nThread; n ++) {
		CELLThread* t = new CELLThread();
		t->Start(
			nullptr, 
			[n](CELLThread *pThread) {
				WorkThread(pThread, n + 1);
			},
			nullptr
		);
		threads.push_back(t);
	}

	//统计信息
	CELLTimestamp tTime;
	while(true) {
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0) {
			printf("thread<%d>, clients<%d>, connect<%d>, time<%lf>, send<%d> \n", nThread, nThread * nClient, (int)nConnect, t, (int)sendCount);
			Log::INFO("thread<%d>, clients<%d>, connect<%d>, time<%lf>, send<%d> \n", nThread, nThread * nClient, (int)nConnect, t, (int)sendCount);
			sendCount = 0;
			tTime.update();
		}
		CELLThread::Sleep(1);
	}

	for (auto t : threads) {
		t->Close();
		delete t;
	}

	Log::INFO("--------------------server is finish--------------------\n");
	return 0;
}
