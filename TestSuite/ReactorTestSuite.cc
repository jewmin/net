#include "gtest/gtest.h"
#include "Common/Allocator.h"
#include "Reactor/EventHandler.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketAcceptor.h"
#include "Reactor/SocketConnector.h"
#include "Reactor/SocketConnection.h"
#include "Sockets/StreamSocketImpl.h"

class MockEventHandler : public Net::EventHandler {
public:
	MockEventHandler(Net::EventReactor * reactor) : Net::EventHandler(reactor) {}
	virtual bool RegisterToReactor() { Duplicate(); return true; }
	virtual bool UnRegisterFromReactor() { Release(); return true; }
};

class MockEventHandler2 : public Net::EventHandler {
public:
	MockEventHandler2(Net::EventReactor * reactor) : Net::EventHandler(reactor) {}
	virtual bool RegisterToReactor() { return false; }
	virtual bool UnRegisterFromReactor() { return false; }
};

class ReactorTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		reactor_ = new Net::EventReactor();
		handler_ = new MockEventHandler(nullptr);
		handler2_ = new MockEventHandler2(reactor_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		handler_->Destroy();
		handler2_->Destroy();
		delete reactor_;
	}

	static void close_cb(uv_handle_t* handle) {
		jc_free(handle);
	}

	static void timer_cb(uv_timer_t * handle) {
		uv_close(reinterpret_cast<uv_handle_t *>(handle), close_cb);
	}

	MockEventHandler * handler_;
	MockEventHandler2 * handler2_;
	Net::EventReactor * reactor_;
};

TEST_F(ReactorTestSuite, event_handler) {
	EXPECT_TRUE(handler_->GetReactor() == nullptr);
	EXPECT_TRUE(handler2_->GetReactor() == reactor_);
	handler_->SetReactor(reactor_);
	handler2_->SetReactor(nullptr);
	EXPECT_TRUE(handler_->GetReactor() == reactor_);
	EXPECT_TRUE(handler2_->GetReactor() == nullptr);
}

TEST_F(ReactorTestSuite, event_handler2) {
	EXPECT_EQ(reactor_->AddEventHandler(handler_), true);
	EXPECT_EQ(reactor_->RemoveEventHandler(handler_), true);
	EXPECT_EQ(reactor_->AddEventHandler(handler2_), false);
	EXPECT_EQ(reactor_->RemoveEventHandler(handler2_), false);
}

TEST_F(ReactorTestSuite, event_handler3) {
	EXPECT_EQ(reactor_->AddEventHandler(handler_), true);
}

TEST_F(ReactorTestSuite, event_handler4) {
	handler_->Duplicate();
	EXPECT_EQ(reactor_->RemoveEventHandler(handler_), true);
}

TEST_F(ReactorTestSuite, reactor) {
	EXPECT_EQ(reactor_->Poll(), false);
}

TEST_F(ReactorTestSuite, reactor2) {
	uv_timer_t * timer = static_cast<uv_timer_t *>(jc_malloc(sizeof(*timer)));
	uv_timer_init(reactor_->GetUvLoop(), timer);
	uv_timer_start(timer, timer_cb, 10, 0);
}

class MockConnection : public Net::SocketConnection {
public:
	MockConnection() : Net::SocketConnection(1024, 1024), call_connected_(0), call_connect_failed_(0)
		, call_disconnected_(0), call_recv_(0), call_sent_(0), call_error_(0) {}
	virtual void OnConnected() {
		Net::SocketConnection::OnConnected();
		call_connected_++;
	}
	virtual void OnConnectFailed(i32 reason) {
		Net::SocketConnection::OnConnectFailed(reason);
		call_connect_failed_++;
		std::printf("OnConnectFailed: %s\n", uv_err_name(reason));
	}
	virtual void OnDisconnected(bool is_remote) {
		Net::SocketConnection::OnDisconnected(is_remote);
		call_disconnected_++;
	}
	virtual void OnNewDataReceived() {
		Net::SocketConnection::OnNewDataReceived();
		call_recv_++;
	}
	virtual void OnSomeDataSent() {
		Net::SocketConnection::OnSomeDataSent();
		call_sent_++;
	}
	virtual void OnError(i32 reason) {
		Net::SocketConnection::OnError(reason);
		call_error_++;
		std::printf("OnError: %s\n", uv_err_name(reason));
	}

	i32 call_connected_;
	i32 call_connect_failed_;
	i32 call_disconnected_;
	i32 call_recv_;
	i32 call_sent_;
	i32 call_error_;
};

class ConnectionTestSuite : public ReactorTestSuite {
public:
	ConnectionTestSuite() {
		std::strncpy(w_content_, "hello world", std::strlen("hello world"));
		w_content_len_ = static_cast<i32>(std::strlen("hello world"));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		ReactorTestSuite::SetUp();
		connection_ = new MockConnection();
		connection_->SetReactor(reactor_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		connection_->Destroy();
		ReactorTestSuite::TearDown();
	}

	i8 w_content_[12];
	i32 w_content_len_;
	MockConnection * connection_;
};

TEST_F(ConnectionTestSuite, state) {
	EXPECT_EQ(connection_->GetConnectState(), Net::ConnectState::kDisconnected);
}

TEST_F(ConnectionTestSuite, socket) {
	Net::StreamSocket other_socket;
	Net::StreamSocket * socket = connection_->GetSocket();
	EXPECT_NE(*socket, other_socket);
	connection_->SetSocket(other_socket);
	socket = connection_->GetSocket();
	EXPECT_EQ(*socket, other_socket);
}

TEST_F(ConnectionTestSuite, read) {
	EXPECT_ANY_THROW(connection_->GetRecvData(w_content_len_));
}

TEST_F(ConnectionTestSuite, read2) {
	EXPECT_ANY_THROW(connection_->PopRecvData(w_content_len_));
}

TEST_F(ConnectionTestSuite, call) {
	connection_->CallOnConnected();
	connection_->CallOnConnected();
	connection_->CallOnConnected();
	connection_->CallOnConnectFailed(UV_ECANCELED);
	connection_->CallOnConnectFailed(UV_EBADF);
	connection_->CallOnConnectFailed(UV_EBUSY);
	EXPECT_EQ(connection_->call_connected_, 1);
	EXPECT_EQ(connection_->call_connect_failed_, 1);
	EXPECT_EQ(connection_->call_disconnected_, 0);
}

TEST_F(ConnectionTestSuite, establish) {
	EXPECT_EQ(connection_->Establish(), false);
}

TEST_F(ConnectionTestSuite, shutdown) {
	connection_->Shutdown(false);
	EXPECT_EQ(connection_->call_connected_, 0);
	EXPECT_EQ(connection_->call_connect_failed_, 0);
	EXPECT_EQ(connection_->call_disconnected_, 0);
}

TEST_F(ConnectionTestSuite, write) {
	EXPECT_EQ(connection_->Write(w_content_, w_content_len_), UV_ENOTCONN);
}

TEST_F(ConnectionTestSuite, read3) {
	EXPECT_EQ(connection_->Read(w_content_, w_content_len_), UV_ENOTCONN);
}

class MockAcceptor : public Net::SocketAcceptor {
public:
	MockAcceptor(Net::EventReactor * reactor) : Net::SocketAcceptor(reactor)
		, connection_(new MockConnection()), use_null_connection_(false) {}
	virtual ~MockAcceptor() { connection_->Destroy(); }
	virtual Net::SocketConnection * CreateConnection() {
		return use_null_connection_ ? nullptr : connection_;
	}
	virtual void AcceptCallback(i32 status) {
		Net::SocketAcceptor::AcceptCallback(status);
	}
	void UseNullConnection(bool use) { use_null_connection_ = use; }
	Net::SocketConnection * connection_;
	bool use_null_connection_;
};

class AcceptorTestSuite : public ConnectionTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectionTestSuite::SetUp();
		acceptor_ = new MockAcceptor(reactor_);
		acceptor_address_ = Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		acceptor_->Destroy();
		ConnectionTestSuite::TearDown();
	}

	MockAcceptor * acceptor_;
	Net::SocketAddress acceptor_address_;
};

TEST_F(AcceptorTestSuite, open) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
}

TEST_F(AcceptorTestSuite, open2) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
	EXPECT_EQ(acceptor_->Open(acceptor_address_), false);
}

TEST_F(AcceptorTestSuite, open3) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_, 128, true), false);
}

TEST_F(AcceptorTestSuite, close) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
	acceptor_->Close();
}

TEST_F(AcceptorTestSuite, close2) {
	acceptor_->Close();
}

TEST_F(AcceptorTestSuite, cb) {
	acceptor_->AcceptCallback(UV_ECANCELED);
}

TEST_F(AcceptorTestSuite, cb2) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
	acceptor_->AcceptCallback(0);
}

class MockStreamSocketImpl : public Net::StreamSocketImpl {
public:
	void delayed_error(i32 err) {
		reinterpret_cast<uv_tcp_t *>(handle_)->delayed_error = err;
	}
};

class MockStreamSocket : public Net::StreamSocket {
public:
	MockStreamSocket() : Net::StreamSocket(new MockStreamSocketImpl()) {}
};

class ConnectorTestSuite : public AcceptorTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		AcceptorTestSuite::SetUp();
		connector_ = new Net::SocketConnector(reactor_);
		client_ = new MockConnection();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		client_->Destroy();
		connector_->Destroy();
		AcceptorTestSuite::TearDown();
	}

	void Poll(i32 count = 30) {
		while (count-- > 0) {
			reactor_->Poll();
		}
	}

	Net::SocketConnection * client_;
	Net::SocketConnector * connector_;
};

TEST_F(ConnectorTestSuite, handler) {
	EXPECT_EQ(reactor_->AddEventHandler(connector_), false);
	EXPECT_EQ(reactor_->RemoveEventHandler(connector_), false);
}

TEST_F(ConnectorTestSuite, connect) {
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);
	EXPECT_EQ(client_->GetConnectState(), Net::ConnectState::kConnecting);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 0);
}

TEST_F(ConnectorTestSuite, connect2) {
	MockStreamSocket socket;
	socket.Open(reactor_->GetUvLoop());
	client_->SetSocket(socket);
#ifdef _WIN32
	dynamic_cast<MockStreamSocketImpl *>(socket.Impl())->delayed_error(UV_EALREADY);
#else
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);
#endif
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), false);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 0);
}

TEST_F(ConnectorTestSuite, cb) {
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);
	client_->GetSocket()->Close();
	Poll();
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 0);
}

class EventLoopTestSuite : public ConnectorTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectorTestSuite::SetUp();
		EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
		EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		ConnectorTestSuite::TearDown();
	}
};

TEST_F(EventLoopTestSuite, cb) {
	Poll();
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_connected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_connect_failed_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_disconnected_, 0);
}

// connection未与socket关联，socket会自动关闭，导致对端返回EOF
TEST_F(EventLoopTestSuite, cb2) {
	acceptor_->UseNullConnection(true);
	Poll();
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_connected_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_connect_failed_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_disconnected_, 0);
}

TEST_F(EventLoopTestSuite, cb3) {
	EXPECT_EQ(connector_->Connect(connection_, acceptor_address_), true);
	Poll();
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 0);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_connected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_connect_failed_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(dynamic_cast<MockAcceptor *>(acceptor_)->connection_)->call_disconnected_, 0);
	EXPECT_EQ(connection_->call_connected_, 1);
	EXPECT_EQ(connection_->call_connect_failed_, 0);
	EXPECT_EQ(connection_->call_disconnected_, 0);
}

TEST_F(EventLoopTestSuite, cb4) {
	Poll();
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), false);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 1);
	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 0);
}

TEST_F(EventLoopTestSuite, cb5) {
	Poll();
	EXPECT_EQ(acceptor_->connection_->GetConnectState(), Net::ConnectState::kConnected);
	EXPECT_EQ(client_->GetConnectState(), Net::ConnectState::kConnected);
	client_->SetSocket(Net::StreamSocket());
	Poll();
	EXPECT_EQ(acceptor_->connection_->GetConnectState(), Net::ConnectState::kDisconnected);
	EXPECT_EQ(client_->GetConnectState(), Net::ConnectState::kConnected);
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);
	EXPECT_EQ(acceptor_->connection_->GetConnectState(), Net::ConnectState::kDisconnected);
	EXPECT_EQ(client_->GetConnectState(), Net::ConnectState::kConnecting);
	EXPECT_ANY_THROW(Poll());
	/*client_->Shutdown(true);
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);*/
}

// class EventLoopConnectionTestSuite : public EventLoopTestSuite {
// public:
// 	// Sets up the test fixture.
// 	virtual void SetUp() {
// 		EventLoopTestSuite::SetUp();
// 		Poll();
// 	}

// 	// Tears down the test fixture.
// 	virtual void TearDown() {
// 		EventLoopTestSuite::TearDown();
// 	}
// };

// TEST_F(EventLoopConnectionTestSuite, establish) {
// 	client_->SetConnectState(Net::ConnectState::kConnecting);
// 	EXPECT_ANY_THROW(client_->Establish());
// }

// TEST_F(EventLoopConnectionTestSuite, shutdown) {
// 	client_->Shutdown(false);
// 	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 1);
// 	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 0);
// 	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 1);
// }

// TEST_F(EventLoopConnectionTestSuite, shutdown2) {
// 	client_->Shutdown(false);
// 	client_->SetConnectState(Net::ConnectState::kConnected);
// 	client_->Shutdown(true);
// 	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connected_, 1);
// 	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_connect_failed_, 0);
// 	EXPECT_EQ(dynamic_cast<MockConnection *>(client_)->call_disconnected_, 1);
// }