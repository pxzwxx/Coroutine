#include "CELL.hpp"
#include "EasyTcpServer.hpp"

int main(int argc, char* args[]) {
	Log::GetLogInstance()->init("./serverLog",100000);
	Log::INFO("--------------------server is start--------------------\n");

	if (argc < 5) Log::ERROR("main parameters error, argc = %d\n", argc);	
	
	//解析ip, port, 子服务数， 最多允许接入的客户端数量
	const char* strIP = args[1];
	uint16_t nPort = (uint16_t)std::stoi(args[2]);
	int nThread = std::stoi(args[3]);
	int nClient = std::stoi(args[4]);

	if (0 == strcmp(strIP, "any")) strIP = nullptr; //监控主机所有ip

	EasyTcpServer server;
	server.setMaxClient(nClient);
	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(10000);
	server.Start(nThread);

	//在主线程中等待用户输入命令
	server.OnRun(); //打印统计信息

	Log::INFO("--------------------server is finish--------------------\n");

	return 0;
}
