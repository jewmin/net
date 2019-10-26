#include "gtest/gtest.h"
#include "Interface/Interface.h"
#include <functional>

#pragma pack(1)
struct InterfaceTestSuiteMsgHeader {
	u8 tag_begin;
	u16 crc_data;
	u8 data;
	u8 tag_end;
};
#pragma pack()

class InterfaceTestSuite : public testing::Test {
public:
	static void MyOnUpdateFunc() {
		if (instance) {
			instance->OnUpdateFunc();
		}
	}

	static void MyOnSignalFunc(int signum) {
		if (instance) {
			instance->OnSignalFunc(signum);
		}
	}
	static void MyOnLogFunc(int level, const char * msg) {
		if (instance) {
			instance->OnLogFunc(level, msg);
		}
	}
	static void MyOnConnectedFunc(u64 mgrId, u32 id) {
		if (instance) {
			instance->OnConnectedFunc(mgrId, id);
		}
	}
	static void MyOnConnectFailedFunc(u64 mgrId, u32 id, int reason) {
		if (instance) {
			instance->OnConnectFailedFunc(mgrId, id, reason);
		}
	}
	static void MyOnDisconnectedFunc(u64 mgrId, u32 id, bool isRemote) {
		if (instance) {
			instance->OnDisconnectedFunc(mgrId, id, isRemote);
		}
	}
	static void MyOnRecvMsgFunc(u64 mgrId, u32 id, int msgId, const char * data, int size) {
		if (instance) {
			instance->OnRecvMsgFunc(mgrId, id, msgId, data, size);
		}
	}
	static void MyOnRecvRawMsgFunc(u64 mgrId, u32 id, const char * data, int size) {
		if (instance) {
			instance->OnRecvRawMsgFunc(mgrId, id, data, size);
		}
	}

protected:
	// Sets up the test fixture.
	virtual void SetUp() {
		instance = this;

		data = "hello";
		client_id = 0;
		server_id = 0;
		client_conn_id = 0;
		server_conn_id = 0;
		client_conn_id2 = 0;
		server_conn_id2 = 0;
		quit = false;
		step = 0;
		loop_num = 30;

		header.tag_begin = 0xbf;
		header.tag_end = 0xef;
		header.data = 1 << 4 | 1;
		header.crc_data = (0xbf << 8) | 0xef | sizeof(data);

		Init(MyOnUpdateFunc, MyOnSignalFunc, MyOnLogFunc);
		SetCallback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnRecvRawMsgFunc);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		quit = true;
		Unit();
		instance = nullptr;
	}

	virtual void OnUpdateFunc() {
		printf("OnUpdate\n");
	}

	virtual void OnSignalFunc(int signum) {
		printf("Receive a shutdown signal %d\n", signum);
		quit = true;
	}

	virtual void OnLogFunc(int level, const char * msg) {
		printf("%d %s\n", level, msg);
	}

	virtual void OnConnectedFunc(u64 mgrId, u32 id) {
		if (mgrId == client_id) {
			EXPECT_EQ(id, client_conn_id);
			printf("Successful connection to service %llu, %u\n", mgrId, id);
		} else if (mgrId == server_id) {
			if (server_conn_id > 0) {
				server_conn_id2 = id;
			} else {
				server_conn_id = id;
			}
			printf("Receivead a peer-to-peer connection %llu, %u\n", mgrId, id);
		}
	}

	virtual void OnConnectFailedFunc(u64 mgrId, u32 id, int reason) {
		EXPECT_EQ(mgrId, client_id);
		EXPECT_EQ(id, client_conn_id);
		printf("Failed to connect to the service %llu, %u, %d\n", mgrId, id, reason);
	}

	virtual void OnDisconnectedFunc(u64 mgrId, u32 id, bool isRemote) {
		if (mgrId == client_id) {
			EXPECT_EQ(id, client_conn_id);
			printf("Disconnect from the service %llu, %u\n", mgrId, id);
		} else if (mgrId == server_id) {
			printf("Disconnecting the peer %llu, %u\n", mgrId, id);
		}
	}

	virtual void OnRecvMsgFunc(u64 mgrId, u32 id, int msgId, const char * data, int size) {
		printf("Receive protocol data from the peer %llu, %u, %d, %s, %d\n", mgrId, id, msgId, data, size);
	}

	virtual void OnRecvRawMsgFunc(u64 mgrId, u32 id, const char * data, int size) {
		printf("Receive raw data from the peer %llu, %u, %s, %d\n", mgrId, id, data, size);
	}

	const char * data;
	u64 client_id;
	u64 server_id;
	u32 client_conn_id;
	u32 server_conn_id;
	u32 client_conn_id2;
	u32 server_conn_id2;
	bool quit;
	u16 step;
	u16 loop_num;
	struct InterfaceTestSuiteMsgHeader header;
	static InterfaceTestSuite * instance;
};

InterfaceTestSuite * InterfaceTestSuite::instance = nullptr;

class InterfaceTestSuite2 : public InterfaceTestSuite {
protected:
	virtual void OnConnectedFunc(u64 mgrId, u32 id) {
		SetRawRecv(mgrId, id, true);
		SendRawMsg(mgrId, id, data, sizeof(data));
	}
};

class InterfaceTestSuite3 : public InterfaceTestSuite {
protected:
	virtual void OnConnectedFunc(u64 mgrId, u32 id) {
		SendMsg(mgrId, id, 200, data, sizeof(data));
		SendMsg(mgrId, id, 50000, data, sizeof(data));
		SendMsg(mgrId, id, 70000, data, sizeof(data));
	}
};

class InterfaceTestSuite4 : public InterfaceTestSuite {
protected:
	virtual void SetUp() {
		std::memset(&address, 0, sizeof(address));
		update_count_ = 0;
		InterfaceTestSuite::SetUp();
	}

	virtual void OnUpdateFunc() {
		++update_count_;
		if (update_count_ == 10) {
			u8 msgId = 100;
			u8 dataLen = 20;
			SendRawMsg(client_id, client_conn_id, reinterpret_cast<const char *>(&msgId), 1);
			SendRawMsg(client_id, client_conn_id, reinterpret_cast<const char *>(&dataLen), 1);
		} else if (update_count_ == 20) {
			SendRawMsg(client_id, client_conn_id, data, sizeof(data));
		}
	}

	void OnConnectedFunc(u64 mgrId, u32 id) {
		address_t addr = GetOneConnectionRemoteAddress(mgrId, id);
		if (mgrId == client_id && id == client_conn_id) {
			std::memcpy(&address, &addr, sizeof(address));
			++step;
			switch (step) {
				case 1: {
					header.tag_begin = 0xef;
					SendRawMsg(mgrId, id, reinterpret_cast<const char *>(&header), sizeof(header));
				}
				break;
				case 2: {
					header.tag_begin = 0xbf;
					SendRawMsg(mgrId, id, reinterpret_cast<const char *>(&header), sizeof(header));
				}
				break;
				case 3: {
					u8 msgId = 100;
					u8 dataLen = sizeof(data);
					SendRawMsg(mgrId, id, reinterpret_cast<const char *>(&header), sizeof(header));
					SendRawMsg(mgrId, id, reinterpret_cast<const char *>(&msgId), 1);
					SendRawMsg(mgrId, id, reinterpret_cast<const char *>(&dataLen), 1);
				}
				break;
			}
		}
	}

	void OnDisconnectedFunc(u64 mgrId, u32 id, bool isRemote) {
		printf("Disconnecting the peer %llu, %u, %d\n", mgrId, id, isRemote);
		if (!quit && mgrId == client_id && id == client_conn_id) {
			client_conn_id = ClientConnect(mgrId, address.address, address.port);
		}
	}

private:
	address_t address;
	int update_count_;
};

TEST_F(InterfaceTestSuite, use) {
	server_id = CreateServer("TestServer", 102400, 102400);
	EXPECT_EQ(ServerListen(server_id, "0.0.0.0", 6789), true);
	client_id = CreateClient("TestClient", 102400, 102400);
	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
	while (--loop_num > 0) {
		Loop();
	}
	EXPECT_EQ(EndServer(server_id), true);
	DeleteServer(server_id);
	DeleteClient(client_id);
}

TEST_F(InterfaceTestSuite, error) {
	EXPECT_EQ(ServerListen(0, "0.0.0.0", 6789), false);
	EXPECT_EQ(EndServer(0), false);
}

 TEST_F(InterfaceTestSuite, loop) {
 	server_id = CreateServer("TestServer", 102400, 102400);
 	ServerListen(server_id, "0.0.0.0", 6789);
 	for (int i = 0; i < 9; ++i) {
 		ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
 	}
 	client_id = CreateClient("TestClient", 102400, 102400);
 	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
 	while (--loop_num > 0) {
 		Loop();
 	}
 	ShutdownConnectionNow(client_id, client_conn_id);
 }

 TEST_F(InterfaceTestSuite, shutdown) {
 	server_id = CreateServer("TestServer", 102400, 102400);
 	ServerListen(server_id, "0.0.0.0", 6789);
 	for (int i = 0; i < 9; ++i) {
 		ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
 	}
 	client_id = CreateClient("TestClient", 102400, 102400);
 	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
 	client_conn_id2 = ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
 	while (--loop_num > 0) {
 		Loop();
 	}
 	ShutdownConnectionNow(server_id, server_conn_id);
 	ShutdownConnection(server_id, server_conn_id2);
 	ShutdownConnection(client_id, client_conn_id2);
 	ShutdownAllConnection(server_id);
 	ShutdownAllConnection(client_id);
 }

 TEST_F(InterfaceTestSuite2, sendRaw) {
 	server_id = CreateServer("TestServer", 102400, 102400);
 	ServerListen(server_id, "0.0.0.0", 6789);
 	for (int i = 0; i < 10; ++i) {
 		ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
 	}
 	while (--loop_num > 0) {
 		Loop();
 	}
 }

 TEST_F(InterfaceTestSuite3, send) {
 	server_id = CreateServer("TestServer", 102400, 102400);
 	ServerListen(server_id, "0.0.0.0", 6789);
 	for (int i = 0; i < 10; ++i) {
 		ClientConnect(CreateClient("TestClient", 102400, 102400), "127.0.0.1", 6789);
 	}
 	while (--loop_num > 0) {
 		Loop();
 	}
 }

 TEST_F(InterfaceTestSuite4, sendBreak) {
 	ServerListen(CreateServer("sendBreakServer", 102400, 102400), "0.0.0.0", 6789);
	client_id = CreateClient("sendBreakClient", 102400, 102400);
 	client_conn_id = ClientConnect(client_id, "127.0.0.1", 6789);
 	while (--loop_num > 0) {
 		Loop();
 	}
 }