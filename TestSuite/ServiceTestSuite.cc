#include "gtest/gtest.h"
#include "Interface/AppService.h"
#include "Interface/Client.h"
#include "Interface/Server.h"
#include "Interface/Connection.h"
#include "Interface/ConnectionMgr.h"
#include "Interface/INotification.h"

class MockNotification : public Net::INotification {
public:
	virtual void OnConnected(Net::Connection * connection) {
		call_connected_++;
	}
	virtual void OnConnectFailed(Net::Connection * connection, i32 reason) {
		call_connect_failed_++;
	}
	virtual void OnDisconnected(Net::Connection * connection, bool is_remote) {
		call_disconnected_++;
	}
	virtual void OnNewDataReceived(Net::Connection * connection) {
		call_new_data_received_++;
	}
	virtual void OnSomeDataSent(Net::Connection * connection) {
		call_some_data_sent_++;
	}
	void Reset() {
		call_connected_ = call_connect_failed_ = call_disconnected_ = 0;
		call_new_data_received_ = call_some_data_sent_ = 0;
	}

	i32 call_connected_;
	i32 call_connect_failed_;
	i32 call_disconnected_;
	i32 call_new_data_received_;
	i32 call_some_data_sent_;
};

class MockNotificationCD : public MockNotification {
public:
	virtual void OnConnected(Net::Connection * connection) {
		MockNotification::OnConnected(connection);
		connection->Shutdown(true);
	}
};

class MockNotificationRW : public MockNotification {
public:
	virtual void OnConnected(Net::Connection * connection) {
		MockNotification::OnConnected(connection);
		connection->Write("123", 3);
	}
	virtual void OnNewDataReceived(Net::Connection * connection) {
		MockNotification::OnNewDataReceived(connection);
		connection->Shutdown(false);
	}
	virtual void OnSomeDataSent(Net::Connection * connection) {
		MockNotification::OnSomeDataSent(connection);
		// connection->Shutdown(false);
	}
};

class MockServiceConnection : public Net::Connection {
public:
	MockServiceConnection(Net::ConnectionMgr * mgr) : Net::Connection(mgr, 64, 64) {}
	virtual ~MockServiceConnection() {}
	virtual void OnConnected() override {
		Net::Connection::OnConnected();
	}
	virtual void OnConnectFailed(i32 reason) override {
		Net::Connection::OnConnectFailed(reason);
	}
	virtual void OnDisconnected(bool is_remote) override {
		Net::Connection::OnDisconnected(is_remote);
	}
	virtual void OnNewDataReceived() override {
		Net::Connection::OnNewDataReceived();
	}
	virtual void OnSomeDataSent() override {
		Net::Connection::OnSomeDataSent();
	}
	virtual void OnError(i32 reason) override {
		Net::Connection::OnError(reason);
	}
};

class MockConnectionMgr : public Net::ConnectionMgr {
public:
	MockConnectionMgr(const std::string & name) : Net::ConnectionMgr(name) {}
	virtual ~MockConnectionMgr() {}
	virtual void Register(Net::Connection * connection) {
		Net::ConnectionMgr::Register(connection);
	}
	virtual void UnRegister(Net::Connection * connection) {
		Net::ConnectionMgr::UnRegister(connection);
	}
};

class ConnectionMgrTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		notification_.Reset();
		notification_cd_.Reset();
		notification_rw_.Reset();
		reactor_ = new Net::EventReactor();
		mgr_ = new MockConnectionMgr("testMgr");
		mgr_->SetNotification(&notification_);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete mgr_;
		delete reactor_;
	}
	virtual void Run(i32 count = 30) {
		while (count-- > 0) { reactor_->Poll(); }
	}
	Net::EventReactor * reactor_;
	MockNotification notification_;
	MockNotificationCD notification_cd_;
	MockNotificationRW notification_rw_;
	MockConnectionMgr * mgr_;
};

TEST_F(ConnectionMgrTestSuite, mgr) {
	EXPECT_EQ(mgr_->GetMgrId(), -1);
	EXPECT_STREQ(mgr_->GetName().c_str(), "testMgr");
	EXPECT_EQ(mgr_->GetConnectionCount(), 0u);
	EXPECT_TRUE(mgr_->GetConnection(1) == nullptr);

	mgr_->SetMgrId(100);
	EXPECT_EQ(mgr_->GetMgrId(), 100);
}

TEST_F(ConnectionMgrTestSuite, null_mgr) {
	EXPECT_ANY_THROW(Net::Connection connection(nullptr, 0, 0));
}

TEST_F(ConnectionMgrTestSuite, conn) {
	Net::Connection connection(mgr_, 64, 64);
	EXPECT_TRUE(connection.GetMgr() == mgr_);
	EXPECT_EQ(connection.GetConnectionId(), -1);
	EXPECT_EQ(connection.IsRegister2Mgr(), false);

	connection.SetConnectionId(10);
	EXPECT_EQ(connection.GetConnectionId(), 10);

	connection.SetRegister2Mgr(true);
	EXPECT_EQ(connection.IsRegister2Mgr(), true);
}

class ConnectionTestSuite : public ConnectionMgrTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectionMgrTestSuite::SetUp();
		for (int i = 0; i < 10; i++) {
			mgr_->Register(new Net::Connection(mgr_, 64, 64));
		}
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		ConnectionMgrTestSuite::TearDown();
	}
};

TEST_F(ConnectionTestSuite, shutdown_one) {
	mgr_->ShutDownOneConnection(-1);
	mgr_->ShutDownOneConnection(0);
	mgr_->ShutDownOneConnection(3);
	mgr_->ShutDownOneConnection(6);
	mgr_->ShutDownOneConnection(9);
	mgr_->ShutDownOneConnection(10);
}

TEST_F(ConnectionTestSuite, shutdown_now) {
	mgr_->ShutDownOneConnection(-1, true);
	mgr_->ShutDownOneConnection(0, true);
	mgr_->ShutDownOneConnection(3, true);
	mgr_->ShutDownOneConnection(6, true);
	mgr_->ShutDownOneConnection(9, true);
	mgr_->ShutDownOneConnection(10, true);
}

TEST_F(ConnectionTestSuite, shutdown_all) {
	mgr_->ShutDownAllConnections();
}

TEST_F(ConnectionTestSuite, reg) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	connection->SetConnectionId(20);
	mgr_->Register(connection);
	EXPECT_EQ(connection->GetConnectionId(), 20);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11u);
	EXPECT_TRUE(mgr_->GetConnection(20) == connection);
}

TEST_F(ConnectionTestSuite, double_reg) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	mgr_->Register(connection);
	EXPECT_EQ(connection->GetConnectionId(), 10);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11u);
	EXPECT_TRUE(mgr_->GetConnection(10) == connection);

	mgr_->Register(connection);
	EXPECT_EQ(connection->GetConnectionId(), 10);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11u);
	EXPECT_TRUE(mgr_->GetConnection(10) == connection);
}

TEST_F(ConnectionTestSuite, unreg) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	mgr_->UnRegister(connection);
	mgr_->Update();
	delete connection;
	EXPECT_EQ(mgr_->GetConnectionCount(), 10u);
}

TEST_F(ConnectionTestSuite, unreg_succ) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	mgr_->Register(connection);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11u);
	mgr_->UnRegister(connection);
	mgr_->Update();
	EXPECT_EQ(mgr_->GetConnectionCount(), 10u);
}

TEST_F(ConnectionTestSuite, unreg_throw) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	connection->SetRegister2Mgr(true);
	mgr_->UnRegister(connection);
	EXPECT_ANY_THROW(mgr_->Update());
	delete connection;
	EXPECT_EQ(mgr_->GetConnectionCount(), 10u);
}

TEST_F(ConnectionTestSuite, unreg_null) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	connection->SetConnectionId(20);
	connection->SetRegister2Mgr(true);
	mgr_->UnRegister(connection);
	mgr_->Update();
	EXPECT_EQ(mgr_->GetConnectionCount(), 10u);
	delete connection;
}

TEST_F(ConnectionTestSuite, cb_connected) {
	MockServiceConnection * connection = new MockServiceConnection(mgr_);

	connection->OnConnected();
	EXPECT_EQ(notification_.call_connected_, 1);

	connection->OnDisconnected(true);
	EXPECT_EQ(notification_.call_disconnected_, 1);
}

TEST_F(ConnectionTestSuite, cb_connect_failed) {
	MockServiceConnection * connection = new MockServiceConnection(mgr_);

	mgr_->Register(connection);
	connection->OnConnectFailed(UV_ECANCELED);
	EXPECT_EQ(notification_.call_connect_failed_, 1);
}

TEST_F(ConnectionTestSuite, cb_misc) {
	MockServiceConnection * connection = new MockServiceConnection(mgr_);

	connection->OnNewDataReceived();
	EXPECT_EQ(notification_.call_new_data_received_, 1);

	connection->OnSomeDataSent();
	EXPECT_EQ(notification_.call_some_data_sent_, 1);

	connection->OnError(UV_EOF);
	connection->OnError(UV_ECANCELED);
	delete connection;
}

class ClientTestSuite : public ConnectionTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectionTestSuite::SetUp();
		connector_ = new Net::SocketConnector(reactor_);
		client_ = new Net::Client("client", reactor_, connector_, 64, 64);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete client_;
		delete connector_;
		ConnectionTestSuite::TearDown();
	}
	Net::Client * client_;
	Net::SocketConnector * connector_;
};

TEST_F(ClientTestSuite, connect) {
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), 0);
	EXPECT_EQ(client_->Connect("::1", 6789), 1);
}

TEST_F(ClientTestSuite, connect_not_connector) {
	Net::Client client("connect_not_connector", reactor_, nullptr, 64, 64);
	EXPECT_EQ(client.Connect("127.0.0.1", 6789), 0);
}

#ifdef _WIN32
TEST_F(ClientTestSuite, connect_address_error) {
	EXPECT_EQ(client_->Connect("0.0.0.0", 0), -1);
}

TEST_F(ClientTestSuite, connect_cb) {
	client_->SetNotification(&notification_);
	EXPECT_EQ(client_->Connect("::", 0), -1);
	EXPECT_EQ(notification_.call_connect_failed_, 1);
}
#endif

class ServerTestSuite : public ClientTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ClientTestSuite::SetUp();
		server_ = new Net::Server("server", reactor_, 64, 64);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete server_;
		ClientTestSuite::TearDown();
	}
	virtual void Run(i32 count = 30) {
		while (count-- > 0) { reactor_->Poll(); server_->Update(); client_->Update(); }
	}
	Net::Server * server_;
};

TEST_F(ServerTestSuite, listen) {
	EXPECT_EQ(server_->Listen("0.0.0.0", 0), true);
}

TEST_F(ServerTestSuite, listen_ipv6) {
	EXPECT_EQ(server_->Listen("::", 0, 10, true), true);
}

TEST_F(ServerTestSuite, listen_ipv6_only) {
	EXPECT_EQ(server_->Listen("", 0, 5, true), false);
}

TEST_F(ServerTestSuite, stop) {
	server_->Stop();
}

TEST_F(ServerTestSuite, stop2) {
	EXPECT_EQ(server_->Listen("0.0.0.0", 0), true);
	server_->Stop();
}

TEST_F(ServerTestSuite, loop) {
	server_->SetNotification(&notification_);
	client_->SetNotification(&notification_);
	EXPECT_EQ(server_->Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), 0);
	Run();
	EXPECT_EQ(server_->GetConnectionCount(), 1u);
	EXPECT_EQ(client_->GetConnectionCount(), 1u);
	EXPECT_EQ(notification_.call_connected_, 2);
}

TEST_F(ServerTestSuite, loop_shutdown) {
	server_->SetNotification(&notification_);
	client_->SetNotification(&notification_cd_);
	EXPECT_EQ(server_->Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), 0);
	Run();
	EXPECT_EQ(server_->GetConnectionCount(), 0u);
	EXPECT_EQ(client_->GetConnectionCount(), 0u);
	EXPECT_EQ(notification_.call_connected_, 1);
	EXPECT_EQ(notification_.call_disconnected_, 1);
	EXPECT_EQ(notification_cd_.call_connected_, 1);
	EXPECT_EQ(notification_cd_.call_disconnected_, 1);
}

TEST_F(ServerTestSuite, loop_rw) {
	server_->SetNotification(&notification_rw_);
	client_->SetNotification(&notification_rw_);
	EXPECT_EQ(server_->Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), 0);
	Run();
	EXPECT_EQ(server_->GetConnectionCount(), 0u);
	EXPECT_EQ(client_->GetConnectionCount(), 0u);
	EXPECT_EQ(notification_rw_.call_connected_, 2);
	EXPECT_EQ(notification_rw_.call_disconnected_, 2);
	EXPECT_EQ(notification_rw_.call_some_data_sent_, 2);
	EXPECT_EQ(notification_rw_.call_new_data_received_, 2);
}

class MockAppService : public Net::AppService {
public:
	MockAppService() {}
	virtual ~MockAppService() {}
};

static std::unordered_map<i64, std::set<i64> *> * all_connections_;

class AppServiceTestSuite : public ServerTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ServerTestSuite::SetUp();
		all_connections_ = new std::unordered_map<i64, std::set<i64> *>();
		service_ = new MockAppService();
		service_->SetCallback(OnConnected, OnConnectFailed, OnDisconnected, OnReceived, OnSent);
		service_->SetSignal(OnSignal);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		for (auto & it : *all_connections_) {
			delete it.second;
		}
		delete all_connections_;
		delete service_;
		ServerTestSuite::TearDown();
	}
	static void OnConnected(i64 mgr_id, i64 connection_id) {
		auto it = all_connections_->find(mgr_id);
		if (it == all_connections_->end()) {
			std::set<i64> * new_set = new std::set<i64>();
			new_set->emplace(connection_id);
			all_connections_->insert({ mgr_id, new_set });
		} else {
			it->second->emplace(connection_id);
		}
	}
	static void OnConnectFailed(i64 mgr_id, i64 connection_id, i32 reason) {
		auto it = all_connections_->find(mgr_id);
		if (it != all_connections_->end()) {
			it->second->erase(connection_id);
		}
	}
	static void OnDisconnected(i64 mgr_id, i64 connection_id, bool is_remote) {
		auto it = all_connections_->find(mgr_id);
		if (it != all_connections_->end()) {
			it->second->erase(connection_id);
		}
	}
	static void OnReceived(i64 mgr_id, i64 connection_id, const i8 * data, i32 size) {
		std::printf("connection[%lld, %lld] recv data (%d)\n", mgr_id, connection_id, size);
	}
	static void OnSent(i64 mgr_id, i64 connection_id) {
		std::printf("connection[%lld, %lld] send data success\n", mgr_id, connection_id);
	}
	static void OnSignal(int signum) {
		std::printf("recv signal %d\n", signum);
	}
	Net::AppService * service_;
};

TEST_F(AppServiceTestSuite, instance) {
	EXPECT_TRUE(Net::AppService::Get() == nullptr);
	Net::AppService * service = Net::AppService::CreateInstance();
	EXPECT_TRUE(Net::AppService::Get() == service);
	Net::AppService * service2 = Net::AppService::CreateInstance();
	EXPECT_TRUE(service == service2);
	Net::AppService::ReleaseInstance();
	EXPECT_TRUE(Net::AppService::Get() == nullptr);
	Net::AppService::ReleaseInstance();
}

TEST_F(AppServiceTestSuite, create_server) {
	EXPECT_EQ(service_->CreateServer("create_server", 64, 64), 0);
	EXPECT_EQ(service_->ServerListen(0, "0.0.0.0", 0), true);
	EXPECT_EQ(service_->EndServer(0), true);
	service_->DeleteServer(0);
}

TEST_F(AppServiceTestSuite, server_error) {
	EXPECT_EQ(service_->ServerListen(0, "0.0.0.0", 0), false);
	EXPECT_EQ(service_->EndServer(0), false);
	service_->DeleteServer(0);
}

TEST_F(AppServiceTestSuite, server_type_error) {
	EXPECT_EQ(service_->CreateClient("create_client", 64, 64), 0);
	EXPECT_EQ(service_->ServerListen(0, "0.0.0.0", 0), false);
	EXPECT_EQ(service_->EndServer(0), false);
	service_->DeleteServer(0);
}

TEST_F(AppServiceTestSuite, create_client) {
	EXPECT_EQ(service_->CreateClient("create_client", 64, 64), 0);
	EXPECT_EQ(service_->ClientConnect(0, "127.0.0.1", 6789), 0);
	service_->DeleteClient(0);
}

TEST_F(AppServiceTestSuite, client_error) {
	EXPECT_EQ(service_->ClientConnect(0, "127.0.0.1", 6789), -1);
	service_->DeleteClient(0);
}

TEST_F(AppServiceTestSuite, client_type_error) {
	EXPECT_EQ(service_->CreateServer("create_server", 64, 64), 0);
	EXPECT_EQ(service_->ClientConnect(0, "127.0.0.1", 6789), -1);
	service_->DeleteClient(0);
}

TEST_F(AppServiceTestSuite, shutdown) {
	EXPECT_EQ(service_->CreateServer("create_server", 64, 64), 0);
	EXPECT_EQ(service_->ServerListen(0, "0.0.0.0", 0), true);
	EXPECT_EQ(service_->CreateClient("create_client", 64, 64), 1);
	EXPECT_EQ(service_->ClientConnect(1, "127.0.0.1", 6789), 0);

	std::set<i64> * servers = new std::set<i64>();
	std::set<i64> * clients = new std::set<i64>();
	clients->emplace(0);
	all_connections_->insert({ 0, servers });
	all_connections_->insert({ 1, clients });
	service_->ShutdownAllConnections(0);
	service_->ShutdownConnection(1, 0);
	service_->ShutdownConnectionNow(1, 0);
}

TEST_F(AppServiceTestSuite, shutdown_error) {
	service_->ShutdownAllConnections(0);
	service_->ShutdownConnection(0, 0);
	service_->ShutdownConnectionNow(0, 0);
}

TEST_F(AppServiceTestSuite, write_error) {
	EXPECT_EQ(service_->CreateClient("create_client", 64, 64), 0);
	EXPECT_EQ(service_->ClientConnect(0, "127.0.0.1", 6789), 0);
	EXPECT_EQ(service_->SendData(0, 0, "123", 3), UV_ENOTCONN);
}

TEST_F(AppServiceTestSuite, write_null) {
	EXPECT_EQ(service_->SendData(0, 0, "123", 3), UV_ENOTCONN);
}

TEST_F(AppServiceTestSuite, address_null) {
	EXPECT_EQ(service_->GetConnectionRemoteAddress(0, 0), Net::SocketAddress());
}

class AppServiceLoopTestSuite : public AppServiceTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		AppServiceTestSuite::SetUp();
		server_id_ = service_->CreateServer("create_server", 64, 64);
		client_id_ = service_->CreateClient("create_client", 64, 64);
		EXPECT_EQ(service_->ServerListen(server_id_, "0.0.0.0", 6789), true);
		connection_id_ = service_->ClientConnect(client_id_, "127.0.0.1", 6789);
		EXPECT_EQ(server_id_, 0);
		EXPECT_EQ(client_id_, 1);
		EXPECT_EQ(connection_id_, 0);
		std::set<i64> * servers = new std::set<i64>();
		std::set<i64> * clients = new std::set<i64>();
		clients->emplace(connection_id_);
		all_connections_->insert({ server_id_, servers });
		all_connections_->insert({ client_id_, clients });
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		AppServiceTestSuite::TearDown();
	}
	void Loop(i32 count = 30) {
		while (--count > 0) {
			service_->RunOnce();
		}
	}
	i64 server_id_;
	i64 client_id_;
	i64 connection_id_;
};

TEST_F(AppServiceLoopTestSuite, loop) {
	Loop();
}

TEST_F(AppServiceLoopTestSuite, del) {
	Loop();
	service_->DeleteClient(client_id_);
	Loop();
}

TEST_F(AppServiceLoopTestSuite, write) {
	Loop();
	EXPECT_EQ(service_->SendData(client_id_, connection_id_, "123", 3), 3);
	Loop();
}

TEST_F(AppServiceLoopTestSuite, address) {
	Loop();
	EXPECT_NE(service_->GetConnectionRemoteAddress(client_id_, connection_id_), Net::SocketAddress());
}

static i32 signum_;

class SignalTestSuite : public AppServiceLoopTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		AppServiceLoopTestSuite::SetUp();
		signum_ = 0;
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		AppServiceLoopTestSuite::TearDown();
	}
	static void walk_cb(uv_handle_t* handle, void* arg) {
		if (handle->type == UV_SIGNAL) {
			uv_signal_t * signal = reinterpret_cast<uv_signal_t *>(handle);
			signal->signal_cb(signal, signum_);
		}
	}
};

TEST_F(SignalTestSuite, sigint) {
	signum_ = SIGINT;
	uv_walk(service_->GetReactor()->GetUvLoop(), walk_cb, nullptr);
	Loop();
}

TEST_F(SignalTestSuite, sigterm) {
	signum_ = SIGTERM;
	uv_walk(service_->GetReactor()->GetUvLoop(), walk_cb, nullptr);
	Loop();
}

TEST_F(SignalTestSuite, sigabrt) {
	signum_ = SIGABRT;
	uv_walk(service_->GetReactor()->GetUvLoop(), walk_cb, nullptr);
	Loop();
}