#include "Interface/Interface.h"
#ifdef USE_VLD
#include "vld.h"
#endif

static u64 clientId = 0;
static u32 connId = 0;
static bool quit = false;

void MyOnUpdateFunc() {
	printf("一次Tick\n");
}

void MyOnSignalFunc(int signum) {
	printf("收到关机信号 %d\n", signum);
	quit = true;
}

void MyOnLogFunc(int level, const char * msg) {
	printf("打印日志 %d %s\n", level, msg);
}

void MyOnConnectedFunc(u64 mgrId, u32 id) {
	address_t address = GetOneConnectionRemoteAddress(mgrId, id);
	printf("连接成功 %llu %u %s:%d\n", mgrId, id, address.address, address.port);
	if (mgrId == clientId && id == connId) {
		SendRawMsg(mgrId, id, "raw", sizeof("raw"));
	} else {
		SendMsg(mgrId, id, 123, "hello", sizeof("hello"));
	}
}

void MyOnConnectFailedFunc(u64 mgrId, u32 id, int reason) {
	printf("连接失败 %llu %u %d\n", mgrId, id, reason);
	clientId = mgrId;
	connId = ClientConnect(clientId, "127.0.0.1", 6789);
	SetRawRecv(clientId, connId, true);
}

void MyOnDisconnectedFunc(u64 mgrId, u32 id, bool isRemote) {
	printf("断开连接 %llu %u %d\n", mgrId, id, isRemote);
}

void MyOnRecvMsgFunc(u64 mgrId, u32 id, int msgId, const char * data, int size) {
	printf("收到协议数据 %llu, %u, %d, %s, %d\n", mgrId, id, msgId, data, size);
	ShutdownConnectionNow(mgrId, id);
}

void MyOnRecvRawMsgFunc(u64 mgrId, u32 id, const char * data, int size) {
	printf("收到原始数据 %llu, %u, %s, %d\n", mgrId, id, data, size);
	ShutdownConnection(mgrId, id);
}

int main(int argc, const char * * argv) {
	Init(MyOnUpdateFunc, MyOnSignalFunc, MyOnLogFunc);
	SetCallback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	u64 server = CreateServer("DllServer", 102400, 102400);
	ServerListen(server, "0.0.0.0", 6789);
	u64 client1 = CreateClient("DllClient", 102400, 102400);
	u64 client2 = CreateClient("DllClient", 102400, 102400);
	ClientConnect(client1, "127.0.0.1", 6789);
	ClientConnect(client2, "127.0.0.1", 8888);
	while (!quit) {
		Loop();
	}
	EndServer(server);
	DeleteServer(server);
	DeleteClient(client1);
	ShutdownAllConnection(client2);
	DeleteClient(client2);
	Unit();
	return 0;
}