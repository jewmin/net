#include "gtest/gtest.h"
#include "Core/Client.h"
#include "Core/Server.h"
#include "Core/Connection.h"
#include "Core/ConnectionMgr.h"
#include "Core/INotification.h"

class MockNotification : public Net::INotification {
public:
	virtual i32 OnConnected(Net::Connection * connection) {
		call_connected_++;
		return 0;
	}
	virtual i32 OnConnectFailed(Net::Connection * connection, i32 reason) {
		call_connect_failed_++;
		return 0;
	}
	virtual i32 OnDisconnected(Net::Connection * connection, bool is_remote) {
		call_disconnected_++;
		return 0;
	}
	virtual i32 OnNewDataReceived(Net::Connection * connection) {
		call_new_data_received_++;
		return 0;
	}
	virtual i32 OnSomeDataSent(Net::Connection * connection) {
		call_some_data_sent_++;
		return 0;
	}
	virtual i32 OnUpdate(Net::Connection * connection) {
		call_update_++;
		return 0;
	}
	void Reset() {
		call_connected_ = call_connect_failed_ = call_disconnected_ = 0;
		call_new_data_received_ = call_some_data_sent_ = call_update_ = 0;
	}

	i32 call_connected_;
	i32 call_connect_failed_;
	i32 call_disconnected_;
	i32 call_new_data_received_;
	i32 call_some_data_sent_;
	i32 call_update_;
};

class MockNotificationWR : public MockNotification {
public:
	virtual i32 OnConnected(Net::Connection * connection) {
		call_connected_++;
		connection->Write("123", 3);
		return 0;
	}
	virtual i32 OnConnectFailed(Net::Connection * connection, i32 reason) {
		call_connect_failed_++;
		return 0;
	}
	virtual i32 OnDisconnected(Net::Connection * connection, bool is_remote) {
		call_disconnected_++;
		return 0;
	}
	virtual i32 OnNewDataReceived(Net::Connection * connection) {
		call_new_data_received_++;
		return 1;
	}
	virtual i32 OnSomeDataSent(Net::Connection * connection) {
		call_some_data_sent_++;
		return 1;
	}
};

class MockNotificationCD : public MockNotification {
public:
	virtual i32 OnConnected(Net::Connection * connection) {
		call_connected_++;
		return 1;
	}
	virtual i32 OnConnectFailed(Net::Connection * connection, i32 reason) {
		call_connect_failed_++;
		return 1;
	}
	virtual i32 OnDisconnected(Net::Connection * connection, bool is_remote) {
		call_disconnected_++;
		return 1;
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

class ServiceTestSuiteTest : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		notification_.Reset();
		notification_cd_.Reset();
		notification_wr_.Reset();
		reactor_ = new Net::EventReactor();
		mgr_ = new MockConnectionMgr("testMgr");
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete mgr_;
		delete reactor_;
	}
	void Run(i32 count = 30) {
		while (count-- > 0) { reactor_->Poll(); }
	}
	Net::EventReactor * reactor_;
	MockNotification notification_;
	MockNotificationCD notification_cd_;
	MockNotificationWR notification_wr_;
	MockConnectionMgr * mgr_;
};

TEST_F(ServiceTestSuiteTest, mgr) {
	MockConnectionMgr mgr("test1");
	mgr.SetNotification(&notification_);
	EXPECT_STREQ(mgr.GetName().c_str(), "test1");
	EXPECT_EQ(mgr.GetConnectionCount(), 0);
	EXPECT_TRUE(mgr.GetConnection(123) == nullptr);
}

TEST_F(ServiceTestSuiteTest, conn) {
	Net::Connection connection(mgr_, 64, 64);
	EXPECT_ANY_THROW(Net::Connection connection2(nullptr, 0, 0));
	EXPECT_TRUE(connection.GetMgr() == mgr_);
	EXPECT_EQ(connection.GetConnectionId(), -1);
	connection.SetConnectionId(10);
	EXPECT_EQ(connection.GetConnectionId(), 10);
	EXPECT_EQ(connection.IsRegister2Mgr(), false);
	connection.SetRegister2Mgr(true);
	EXPECT_EQ(connection.IsRegister2Mgr(), true);
}

TEST_F(ServiceTestSuiteTest, update) {
	mgr_->Update();
}

class ConnectionMgrTestSuiteTest : public ServiceTestSuiteTest {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ServiceTestSuiteTest::SetUp();
		for (int i = 0; i < 10; i++) {
			mgr_->Register(new Net::Connection(mgr_, 64, 64));
		}
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		ServiceTestSuiteTest::TearDown();
	}
};

TEST_F(ConnectionMgrTestSuiteTest, reg) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	mgr_->Register(connection);
	EXPECT_EQ(connection->GetConnectionId(), 10);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11);
	EXPECT_TRUE(mgr_->GetConnection(10) == connection);
}

TEST_F(ConnectionMgrTestSuiteTest, reg2) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	connection->SetConnectionId(20);
	mgr_->Register(connection);
	EXPECT_EQ(connection->GetConnectionId(), 20);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11);
	EXPECT_TRUE(mgr_->GetConnection(20) == connection);
	mgr_->Register(connection);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11);
}

TEST_F(ConnectionMgrTestSuiteTest, unreg) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	mgr_->UnRegister(connection);
	delete connection;
	EXPECT_EQ(mgr_->GetConnectionCount(), 10);
}

TEST_F(ConnectionMgrTestSuiteTest, unreg2) {
	Net::Connection * connection = new Net::Connection(mgr_, 64, 64);
	mgr_->Register(connection);
	EXPECT_EQ(mgr_->GetConnectionCount(), 11);
	mgr_->UnRegister(connection);
	EXPECT_EQ(mgr_->GetConnectionCount(), 10);
}

TEST_F(ConnectionMgrTestSuiteTest, shutdown) {
	mgr_->ShutDownOneConnection(20);
	mgr_->ShutDownOneConnection(5);
}

TEST_F(ConnectionMgrTestSuiteTest, shutdown2) {
	mgr_->ShutDownAllConnections();
	mgr_->ShutDownOneConnection(5);
}

class ClientTestSuiteTest : public ConnectionMgrTestSuiteTest {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectionMgrTestSuiteTest::SetUp();
		connector_ = new Net::SocketConnector(reactor_);
		client_ = new Net::Client("client", reactor_, connector_, 64, 64);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete client_;
		delete connector_;
		ConnectionMgrTestSuiteTest::TearDown();
	}
	Net::Client * client_;
	Net::SocketConnector * connector_;
};

TEST_F(ClientTestSuiteTest, connect) {
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), true);
	EXPECT_EQ(client_->Connect("::1", 6789), true);
}

TEST_F(ClientTestSuiteTest, connect2) {
	Net::Client * client = nullptr;
	{
		Net::SocketConnector connector(reactor_);
		client = new Net::Client("err client", reactor_, &connector, 64, 64);
	}
	EXPECT_EQ(client->Connect("127.0.0.1", 6789), false);
	delete client;
}

TEST_F(ClientTestSuiteTest, connect3) {
	EXPECT_EQ(client_->Connect("::1", 0), false);
}

TEST_F(ClientTestSuiteTest, connect4) {
	client_->SetNotification(&notification_);
	EXPECT_EQ(client_->Connect("::1", 0), false);
	EXPECT_EQ(notification_.call_connect_failed_, 1);
}

class ServerTestSuiteTest : public ClientTestSuiteTest {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ClientTestSuiteTest::SetUp();
		server_ = new Net::Server("server", reactor_, 64, 64);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete server_;
		ClientTestSuiteTest::TearDown();
	}
	Net::Server * server_;
};

TEST_F(ServerTestSuiteTest, listen) {
	EXPECT_EQ(server_->Listen("0.0.0.0", 0), true);
}

TEST_F(ServerTestSuiteTest, listen2) {
	EXPECT_EQ(server_->Listen("::", 0, 10, true), true);
}

TEST_F(ServerTestSuiteTest, listen3) {
	EXPECT_EQ(server_->Listen("", 0, 5, true), false);
}

TEST_F(ServerTestSuiteTest, loop) {
	server_->SetNotification(&notification_);
	client_->SetNotification(&notification_);
	EXPECT_EQ(server_->Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), true);
	Run();
	EXPECT_EQ(server_->GetConnectionCount(), 1);
	EXPECT_EQ(client_->GetConnectionCount(), 1);
	EXPECT_EQ(notification_.call_connected_, 2);
}

TEST_F(ServerTestSuiteTest, loop2) {
	server_->SetNotification(&notification_);
	client_->SetNotification(&notification_cd_);
	EXPECT_EQ(server_->Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), true);
	Run();
	EXPECT_EQ(server_->GetConnectionCount(), 0);
	EXPECT_EQ(client_->GetConnectionCount(), 0);
	EXPECT_EQ(notification_.call_connected_, 1);
	EXPECT_EQ(notification_.call_disconnected_, 1);
	EXPECT_EQ(notification_cd_.call_connected_, 1);
	EXPECT_EQ(notification_cd_.call_disconnected_, 1);
}

TEST_F(ServerTestSuiteTest, loop3) {
	server_->SetNotification(&notification_wr_);
	client_->SetNotification(&notification_wr_);
	EXPECT_EQ(server_->Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client_->Connect("127.0.0.1", 6789), true);
	Run();
	EXPECT_EQ(server_->GetConnectionCount(), 0);
	EXPECT_EQ(client_->GetConnectionCount(), 0);
	EXPECT_EQ(notification_wr_.call_connected_, 2);
	EXPECT_EQ(notification_wr_.call_disconnected_, 2);
	EXPECT_EQ(notification_wr_.call_some_data_sent_, 2);
	EXPECT_EQ(notification_wr_.call_new_data_received_, 1);
}