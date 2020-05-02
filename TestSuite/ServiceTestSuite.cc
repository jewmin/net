#include "gtest/gtest.h"
#include "Core/Client.h"
#include "Core/Server.h"
#include "Core/Connection.h"
#include "Core/ConnectionMgr.h"
#include "Core/INotification.h"

class ServiceTestSuite_MockNotification : public Net::INotification {
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

class ServiceTestSuite_MockNotification2 : public Net::INotification {
public:
	virtual i32 OnConnected(Net::Connection * connection) {
		return 1;
	}
	virtual i32 OnConnectFailed(Net::Connection * connection, i32 reason) {
		return 1;
	}
	virtual i32 OnDisconnected(Net::Connection * connection, bool is_remote) {
		return 1;
	}
	virtual i32 OnNewDataReceived(Net::Connection * connection) {
		return 1;
	}
	virtual i32 OnSomeDataSent(Net::Connection * connection) {
		return 1;
	}
	virtual i32 OnUpdate(Net::Connection * connection) {
		return 1;
	}
};

class ServiceTestSuite_MockConnectionMgr : public Net::ConnectionMgr {
public:
	ServiceTestSuite_MockConnectionMgr(const std::string & name, u32 object_max_count)
		: Net::ConnectionMgr(name, object_max_count) {}
	virtual ~ServiceTestSuite_MockConnectionMgr() {}
};

class ServiceTestSuiteTest : public testing::Test {
public:
	void SetUp() override { notification_.Reset(); }
	void TearDown() override {}
	void Run(i32 count = 10) { while (count-- > 0) { reactor_.Poll(); } }
	Net::EventReactor reactor_;
	ServiceTestSuite_MockNotification notification_;
	ServiceTestSuite_MockNotification2 notification2_;
};

TEST_F(ServiceTestSuiteTest, testMgr) {
	Net::Connection * connection = nullptr;
	Net::ConnectionMgr * mgr = new ServiceTestSuite_MockConnectionMgr("testMgr", 64);
	mgr->SetNotification(&notification_);
	for (i32 i = 0; i < 64; ++i) {
		connection = new Net::Connection(mgr, 64, 64);
		connection->OnConnected();
		connection->OnNewDataReceived();
		connection->OnSomeDataSent();
		EXPECT_EQ(mgr->GetConnectionCount(), i + 1);
	}
	connection = new Net::Connection(mgr, 64, 64);
	connection->OnConnected();
	connection->OnConnectFailed(UV_ECANCELED);
	EXPECT_EQ(mgr->GetConnectionCount(), 64);
	EXPECT_STREQ(mgr->GetName().c_str(), "testMgr");
	connection = mgr->GetConnection(32);
	EXPECT_EQ(connection->IsRegister2Mgr(), true);
	EXPECT_EQ(connection->GetConnectionId(), 32);
	EXPECT_TRUE(connection->GetMgr() == mgr);
	bool is_remote = false;
	for (i32 i = 1; i < 64; i += 2) {
		is_remote = !is_remote;
		connection = mgr->GetConnection(i);
		connection->OnDisconnected(is_remote);
	}
	mgr->ShutDownOneConnection(0);
	mgr->ShutDownAllConnections();
	mgr->Update();
	i32 error_type[10] = { UV_EISCONN, UV_ENOBUFS, UV_ENOMEM, UV_ENOTSUP, UV_ENOSYS, UV_ECANCELED, UV_EINVAL, UV_EFAULT, UV_EBUSY, UV_EBADF };
	for (i32 i = 0; i < 64; ++i) {
		connection = mgr->GetConnection(i);
		if (connection) {
			connection->OnError(error_type[i % 10]);
		}
	}
	EXPECT_EQ(notification_.call_connected_, 64);
	EXPECT_EQ(notification_.call_connect_failed_, 1);
	EXPECT_EQ(notification_.call_disconnected_, 32);
	EXPECT_EQ(notification_.call_new_data_received_, 64);
	EXPECT_EQ(notification_.call_some_data_sent_, 64);
	EXPECT_EQ(notification_.call_update_, 0);
	delete mgr;
}

TEST_F(ServiceTestSuiteTest, testMgrError) {
	Net::Connection * connection = nullptr;
	Net::ConnectionMgr * mgr = new ServiceTestSuite_MockConnectionMgr("testMgrError", 64);
	mgr->SetNotification(&notification2_);
	for (i32 i = 0; i < 64; ++i) {
		connection = new Net::Connection(mgr, 64, 64);
		connection->OnConnected();
		connection->OnNewDataReceived();
		connection->OnSomeDataSent();
		EXPECT_EQ(mgr->GetConnectionCount(), i + 1);
	}
	delete mgr;
}

TEST_F(ServiceTestSuiteTest, testConnection) {
	Net::Server server("testServer", &reactor_, 128, 128, 64);
	Net::Client client("testClient", &reactor_, nullptr, 128, 128, 64);
	server.SetNotification(&notification_);
	client.SetNotification(&notification_);
	EXPECT_EQ(server.Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(client.Connect("127.0.0.1", 6789), true);
	Run();
	EXPECT_EQ(notification_.call_connected_, 2);
}

TEST_F(ServiceTestSuiteTest, testIPv6) {
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	Net::Server server("testServer", &reactor_, 128, 128, 64);
	Net::Client client("testClient", &reactor_, connector, 128, 128, 64);
	server.SetNotification(&notification_);
	client.SetNotification(&notification_);
	EXPECT_EQ(server.Listen("::", 6789, 128, true), true);
	EXPECT_EQ(client.Connect("::1", 6789), true);
	Run();
	connector->Destroy();
	EXPECT_EQ(notification_.call_connected_, 2);
}

TEST_F(ServiceTestSuiteTest, testConnectError) {
	Net::Server server("testConnectErrorServer", &reactor_, 128, 128, 64);
	Net::Client client("testConnectErrorClient", &reactor_, nullptr, 128, 128, 64);
	Net::Client client2("testConnectErrorClient2", &reactor_, nullptr, 128, 128, 64);
	EXPECT_EQ(server.Listen("::", 6789, 128, true), true);
	EXPECT_EQ(client.Connect("127.0.0.1", 6789), true);
	EXPECT_EQ(client.Connect("::1", 6789), true);
	EXPECT_EQ(client2.Connect("192.168.1.1", 6789), true);
	Run();
}

TEST_F(ServiceTestSuiteTest, testConnectRepeat) {
	Net::Server server("testConnectRepeatServer", &reactor_, 128, 128, 64);
	Net::Client client("testConnectRepeatClient", &reactor_, nullptr, 128, 128, 64);
	server.SetNotification(&notification_);
	client.SetNotification(&notification_);
	EXPECT_EQ(server.Listen("::", 6789), true);
	EXPECT_EQ(client.Connect("127.0.0.1", 6789), true);
	EXPECT_EQ(client.Connect("::1", 6789), true);
	Run();
	EXPECT_EQ(notification_.call_connected_, 4);
}