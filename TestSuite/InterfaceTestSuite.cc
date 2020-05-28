#include "gtest/gtest.h"
#include "Net.h"

class InterfaceTestSuite : public testing::Test {
public:
	static void MyOnSignalFunc(int signum) {
		if (instance) {
			instance->OnSignalFunc(signum);
		}
	}
	static void MyOnConnectedFunc(i64 mgr_id, i64 connection_id) {
		if (instance) {
			instance->OnConnectedFunc(mgr_id, connection_id);
		}
	}
	static void MyOnConnectFailedFunc(i64 mgr_id, i64 connection_id, i32 reason) {
		if (instance) {
			instance->OnConnectFailedFunc(mgr_id, connection_id, reason);
		}
	}
	static void MyOnDisconnectedFunc(i64 mgr_id, i64 connection_id, bool is_remote) {
		if (instance) {
			instance->OnDisconnectedFunc(mgr_id, connection_id, is_remote);
		}
	}
	static void MyOnRecvMsgFunc(i64 mgr_id, i64 connection_id, const i8 * data, i32 size) {
		if (instance) {
			instance->OnRecvMsgFunc(mgr_id, connection_id, data, size);
		}
	}
	static void MyOnSendMsgFunc(i64 mgr_id, i64 connection_id) {
		if (instance) {
			instance->OnSendMsgFunc(mgr_id, connection_id);
		}
	}

protected:
	// Sets up the test fixture.
	virtual void SetUp() {
		instance = this;

		data = "hello";
		client_id = -1;
		server_id = -1;
		client_conn_id = -1;
		server_conn_id = -1;
		quit = false;
		step = 0;
		loop_num = 30;

		jc_init();
		jc_replace_signal(MyOnSignalFunc);
		jc_replace_callback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnSendMsgFunc);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		quit = true;
		jc_unit();
		instance = nullptr;
	}

	virtual void OnSignalFunc(int signum) {
		std::printf("Receive a shutdown signal %d\n", signum);
		quit = true;
	}
	virtual void OnConnectedFunc(i64 mgr_id, i64 connection_id) {
		if (mgr_id == client_id) {
			std::printf("Successful connection to service %lld, %lld\n", mgr_id, connection_id);
			EXPECT_EQ(connection_id, client_conn_id);
		} else if (mgr_id == server_id) {
			std::printf("Receivead a peer-to-peer connection %lld, %lld\n", mgr_id, connection_id);
			server_conn_id = connection_id;
		}
	}
	virtual void OnConnectFailedFunc(i64 mgr_id, i64 connection_id, i32 reason) {
		std::printf("Failed to connect to the service %lld, %lld, %d\n", mgr_id, connection_id, reason);
		EXPECT_EQ(mgr_id, client_id);
		EXPECT_EQ(connection_id, client_conn_id);
	}
	virtual void OnDisconnectedFunc(i64 mgr_id, i64 connection_id, bool is_remote) {
		if (mgr_id == client_id) {
			std::printf("Disconnect from the service %lld, %lld\n", mgr_id, connection_id);
			EXPECT_EQ(connection_id, client_conn_id);
		} else if (mgr_id == server_id) {
			std::printf("Disconnecting the peer %lld, %lld\n", mgr_id, connection_id);
		}
	}
	virtual void OnRecvMsgFunc(i64 mgr_id, i64 connection_id, const i8 * data, i32 size) {
		std::printf("Receive protocol data from the peer %lld, %lld, %s, %d\n", mgr_id, connection_id, data, size);
	}
	virtual void OnSendMsgFunc(i64 mgr_id, i64 connection_id) {
		std::printf("Sent protocol data %lld, %lld\n", mgr_id, connection_id);
	}

	const char * data;
	i64 client_id;
	i64 server_id;
	i64 client_conn_id;
	i64 server_conn_id;
	bool quit;
	u16 step;
	u16 loop_num;
	static InterfaceTestSuite * instance;
};

InterfaceTestSuite * InterfaceTestSuite::instance = nullptr;

TEST_F(InterfaceTestSuite, create) {
	server_id = jc_create_server("TestServer", 102400, 102400);
	EXPECT_EQ(jc_server_listen(server_id, "0.0.0.0", 6789), true);
	client_id = jc_create_client("TestClient", 102400, 102400);
	client_conn_id = jc_client_connect(client_id, "127.0.0.1", 6789);
	while (--loop_num > 0) {
		jc_poll();
	}
	EXPECT_EQ(jc_end_server(server_id), true);
	jc_delete_server(server_id);
	jc_delete_client(client_id);
}

TEST_F(InterfaceTestSuite, create_error) {
	EXPECT_EQ(jc_server_listen(0, "0.0.0.0", 6789), false);
	EXPECT_EQ(jc_end_server(0), false);
}

TEST_F(InterfaceTestSuite, loop) {
	server_id = jc_create_server("TestServer", 102400, 102400);
	jc_server_listen(server_id, "0.0.0.0", 6789);
	for (int i = 0; i < 9; ++i) {
		jc_client_connect(jc_create_client("TestClient", 102400, 102400), "127.0.0.1", 6789);
	}
	client_id = jc_create_client("TestClient", 102400, 102400);
	client_conn_id = jc_client_connect(client_id, "127.0.0.1", 6789);
	while (--loop_num > 0) {
		jc_poll();
	}
	jc_shutdown_one_connection_now(client_id, client_conn_id);
}

TEST_F(InterfaceTestSuite, shutdown) {
	server_id = jc_create_server("TestServer", 102400, 102400);
	jc_server_listen(server_id, "0.0.0.0", 6789);
	for (int i = 0; i < 9; ++i) {
		jc_client_connect(jc_create_client("TestClient", 102400, 102400), "127.0.0.1", 6789);
	}
	client_id = jc_create_client("TestClient", 102400, 102400);
	client_conn_id = jc_client_connect(client_id, "127.0.0.1", 6789);
	i64 conn_id = jc_client_connect(jc_create_client("TestClient", 102400, 102400), "127.0.0.1", 6789);
	while (--loop_num > 0) {
		jc_poll();
	}
	jc_shutdown_one_connection_now(server_id, server_conn_id);
	jc_shutdown_one_connection(server_id, -1);
	jc_shutdown_one_connection(client_id, conn_id);
	jc_shutdown_all_connections(server_id);
	jc_shutdown_all_connections(client_id);
}

class InterfaceTestSuite2 : public InterfaceTestSuite {
public:
	virtual void OnConnectedFunc(i64 mgr_id, i64 connection_id) {
		address_t address = jc_get_one_connection_remote_address(mgr_id, connection_id);
		std::printf("connected %s:%d", address.ip, address.port);
		EXPECT_EQ((i32)sizeof(data), jc_send_data(mgr_id, connection_id, data, sizeof(data)));
	}
};

TEST_F(InterfaceTestSuite2, send) {
	server_id = jc_create_server("TestServer", 102400, 102400);
	jc_server_listen(server_id, "0.0.0.0", 6789);
	for (int i = 0; i < 10; ++i) {
		jc_client_connect(jc_create_client("TestClient", 102400, 102400), "127.0.0.1", 6789);
	}
	while (--loop_num > 0) {
		jc_poll();
	}
}

TEST(InterfaceNullTestSuite, error) {
	EXPECT_EQ(jc_create_server("error server", 64, 64), -1);
	EXPECT_EQ(jc_server_listen(0, "127.0.0.1", 6789), false);
	EXPECT_EQ(jc_end_server(0), false);

	EXPECT_EQ(jc_create_client("error client", 64, 64), -1);
	EXPECT_EQ(jc_client_connect(0, "127.0.0.1", 6789), -1);

	EXPECT_EQ(jc_send_data(0, 0, "hello", sizeof("hello")), UV_EPERM);
}