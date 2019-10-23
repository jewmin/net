#include "gtest/gtest.h"
#include "Interface/Interface.h"
#include "uv.h"

static u64 client_id = 0;
static u64 server_id = 0;
static u32 client_conn_id = 0;
static u32 server_conn_id = 0;
static u32 client_conn_id2 = 0;
static u32 server_conn_id2 = 0;
static bool quit = false;

void MyOnSignalFunc(int signum) {
	printf("收到关机信号 %d\n", signum);
	quit = true;
}

void MyOnLogFunc(int level, const char * msg) {
	printf("%d %s\n", level, msg);
}

void MyOnConnectedFunc(u64 mgrId, u32 id) {	
	if (mgrId == client_id) {
		EXPECT_EQ(id, client_conn_id);
		printf("成功连接服务端 %llu, %u\n", mgrId, id);
	} else if (mgrId == server_id) {
		if (server_conn_id > 0) {
			server_conn_id2 = id;
		} else {
			server_conn_id = id;
		}
		printf("收到对端的连接 %llu, %u\n", mgrId, id);
	}
}

void MyOnConnectedFunc2(u64 mgrId, u32 id) {
	SetRawRecv(mgrId, id, true);
	SendRawMsg(mgrId, id, "hello world", sizeof("hello world"));
}

void MyOnConnectedFunc3(u64 mgrId, u32 id) {
	SendMsg(mgrId, id, 200, "hello world", sizeof("hello world"));
	SendMsg(mgrId, id, 50000, "hello world", sizeof("hello world"));
	SendMsg(mgrId, id, 70000, "hello world", sizeof("hello world"));
}

void MyOnConnectFailedFunc(u64 mgrId, u32 id, int reason) {
	EXPECT_EQ(mgrId, client_id);
	EXPECT_EQ(id, client_conn_id);
	printf("连接服务端失败 %llu, %u, %s\n", mgrId, id, uv_strerror(reason));
}

void MyOnDisconnectedFunc(u64 mgrId, u32 id, bool isRemote) {
	if (mgrId == client_id) {
		EXPECT_EQ(id, client_conn_id);
		printf("与服务端 %llu, %u\n", mgrId, id);
	} else if (mgrId == server_id) {
		printf("断开对端的连接 %llu, %u\n", mgrId, id);
	}
}

void MyOnRecvMsgFunc(u64 mgrId, u32 id, int msgId, const char * data, int size) {
	printf("收到对端发来的协议数据 %llu, %u, %d, %s, %d\n", mgrId, id, msgId, data, size);
}

void MyOnRecvRawMsgFunc(u64 mgrId, u32 id, const char * data, int size) {
	printf("收到对端发来的原始数据 %llu, %u, %s, %d\n", mgrId, id, data, size);
}

TEST(InterfaceTestSuite, use) {
	Init(MyOnSignalFunc, MyOnLogFunc);
	SetCallback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	server_id = CreateServer("TestServer", 102400, 102400);
	EXPECT_EQ(ServerListen(server_id, "0.0.0.0", 6789), true);
	Loop();
	client_id = CreateClient("TestClient", 102400, 102400);
	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
	Loop();
	EXPECT_EQ(EndServer(server_id), true);
	DeleteServer(server_id);
	DeleteClient(client_id);
	Unit();
}

TEST(InterfaceTestSuite, error) {
	Init(nullptr, nullptr);
	EXPECT_EQ(false, ServerListen(0, "0.0.0.0", 6789));
	EXPECT_EQ(false, EndServer(0));
	Unit();
}

TEST(InterfaceTestSuite, loop) {
	Init(MyOnSignalFunc, MyOnLogFunc);
	SetCallback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	server_id = CreateServer("TestServer", 102400, 102400);
	ServerListen(server_id, "0.0.0.0", 6789);
	for (int i = 0; i < 9; ++i) {
		ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
	}
	client_id = CreateClient("TestClient", 102400, 102400);
	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
	int loop_num = 50;
	while (--loop_num > 0) {
		Loop();
	}
	ShutdownConnectionNow(client_id, client_conn_id);
	Unit();
}

TEST(InterfaceTestSuite, shutdown) {
	Init(MyOnSignalFunc, MyOnLogFunc);
	SetCallback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	server_id = CreateServer("TestServer", 102400, 102400);
	ServerListen(server_id, "0.0.0.0", 6789);
	for (int i = 0; i < 9; ++i) {
		ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
	}
	client_id = CreateClient("TestClient", 102400, 102400);
	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
	client_conn_id2 = ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
	int loop_num = 50;
	while (--loop_num > 0) {
		Loop();
	}
	ShutdownConnectionNow(server_id, server_conn_id);
	ShutdownConnection(server_id, server_conn_id2);
	ShutdownConnection(client_id, client_conn_id2);
	ShutdownAllConnection(server_id);
	ShutdownAllConnection(client_id);
	Unit();
}

TEST(InterfaceTestSuite, sendRaw) {
	Init(MyOnSignalFunc, MyOnLogFunc);
	SetCallback(MyOnConnectedFunc2, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	server_id = CreateServer("TestServer", 102400, 102400);
	ServerListen(server_id, "0.0.0.0", 6789);
	u64 client_ids[10];
	u32 client_conn_ids[10];
	for (int i = 0; i < 10; ++i) {
		client_ids[i] = CreateClient("TestClient", 102400, 102400);
		client_conn_ids[i] = ClientConnect(client_ids[i], "127.0.0.1", 6789);
	}
	int loop_num = 10;
	while (--loop_num > 0) {
		Loop();
	}
	Unit();
}

TEST(InterfaceTestSuite, send) {
	Init(MyOnSignalFunc, MyOnLogFunc);
	SetCallback(MyOnConnectedFunc3, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	server_id = CreateServer("TestServer", 102400, 102400);
	ServerListen(server_id, "0.0.0.0", 6789);
	u64 client_ids[10];
	u32 client_conn_ids[10];
	for (int i = 0; i < 10; ++i) {
		client_ids[i] = CreateClient("TestClient", 102400, 102400);
		client_conn_ids[i] = ClientConnect(client_ids[i], "127.0.0.1", 6789);
	}
	int loop_num = 10;
	while (--loop_num > 0) {
		Loop();
	}
	Unit();
}