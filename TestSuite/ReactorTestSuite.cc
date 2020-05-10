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
	virtual bool RegisterToReactor() { return true; }
	virtual bool UnRegisterFromReactor() { return true; }
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
		delete handler_;
		delete handler2_;
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
	EXPECT_EQ(reactor_->GetHandlerCount(), 1);
	EXPECT_EQ(reactor_->RemoveEventHandler(handler_), true);
	EXPECT_EQ(reactor_->GetHandlerCount(), 0);
	EXPECT_EQ(reactor_->AddEventHandler(handler2_), false);
	EXPECT_EQ(reactor_->GetHandlerCount(), 0);
	EXPECT_EQ(reactor_->RemoveEventHandler(handler2_), false);
	EXPECT_EQ(reactor_->GetHandlerCount(), 0);
}

TEST_F(ReactorTestSuite, event_handler3) {
	EXPECT_EQ(reactor_->AddEventHandler(handler_), true);
}

TEST_F(ReactorTestSuite, event_handler4) {
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

class MockAcceptor : public Net::SocketAcceptor {
public:
	MockAcceptor(Net::EventReactor * reactor) : Net::SocketAcceptor(reactor)
		, use_null_connection_(false), use_exist_connection_(false) {}
	virtual ~MockAcceptor() {
		for (auto & it : connection_list_) {
			delete it;
		}
	}
	virtual Net::SocketConnection * CreateConnection() {
		if (use_null_connection_) {
			return nullptr;
		} else if (use_exist_connection_ && connection_list_.size() > 0) {
			return connection_list_.front();
		} else {
			Net::SocketConnection * connection = new MockConnection();
			connection_list_.push_back(connection);
			return connection;
		}
	}
	virtual void DestroyConnection(Net::SocketConnection * connection) {
		connection_list_.remove(connection);
		delete connection;
	}
	virtual void AcceptCallback(i32 status) {
		Net::SocketAcceptor::AcceptCallback(status);
	}
	void UseNullConnection(bool use) { use_null_connection_ = use; }
	void UseExistConnection(bool use) { use_exist_connection_ = use; }
	std::list<Net::SocketConnection *> connection_list_;
	bool use_null_connection_;
	bool use_exist_connection_;
};

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

class ReactorPollTestSuite : public ReactorTestSuite {
public:
	ReactorPollTestSuite() {
		std::strncpy(w_content_, "hello world", std::strlen("hello world"));
		w_content_len_ = static_cast<i32>(std::strlen("hello world"));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		ReactorTestSuite::SetUp();
		connection_ = new MockConnection();
		acceptor_ = new MockAcceptor(reactor_);
		connection_->SetReactor(reactor_);
		connector_ = new Net::SocketConnector(reactor_);
		client_ = new MockConnection();
		acceptor_address_ = Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789);
		EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
		EXPECT_EQ(connector_->Connect(connection_, acceptor_address_), true);
		Poll();
		EXPECT_EQ(connection_->GetConnectState(), Net::ConnectState::kConnected);
		EXPECT_EQ(acceptor_->connection_list_.front()->Write(w_content_, w_content_len_), w_content_len_);
		EXPECT_EQ(connector_->Connect(client_, acceptor_address_), true);
		Poll();
		EXPECT_EQ(client_->GetConnectState(), Net::ConnectState::kConnected);
		EXPECT_EQ(connection_->call_connected_, 1);
		EXPECT_EQ(connection_->call_recv_, 1);
		EXPECT_EQ(client_->call_connected_, 1);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete connection_;
		delete acceptor_;
		delete connector_;
		delete client_;
		ReactorTestSuite::TearDown();
	}

	void Poll(i32 count = 30) {
		while (count-- > 0) {
			reactor_->Poll();
		}
	}

	i8 w_content_[12];
	i32 w_content_len_;
	MockConnection * connection_;
	MockConnection * client_;
	MockAcceptor * acceptor_;
	Net::SocketConnector * connector_;
	Net::SocketAddress acceptor_address_;
};

TEST_F(ReactorPollTestSuite, ctor) {
	MockAcceptor acceptor(reactor_);
	Net::SocketConnector connector(reactor_);
	MockConnection connection;
}

TEST_F(ReactorPollTestSuite, handler) {
	MockAcceptor acceptor(reactor_);
	MockConnection connection;
	Net::SocketConnector connector(reactor_);
	EXPECT_EQ(reactor_->AddEventHandler(&acceptor), true);
	EXPECT_EQ(reactor_->RemoveEventHandler(&acceptor), true);
	EXPECT_EQ(reactor_->AddEventHandler(&connector), false);
	EXPECT_EQ(reactor_->RemoveEventHandler(&connector), false);
	EXPECT_EQ(reactor_->AddEventHandler(&connection), false);
	EXPECT_EQ(reactor_->RemoveEventHandler(&connection), false);
}

TEST_F(ReactorPollTestSuite, open) {
	MockAcceptor acceptor(reactor_);
	EXPECT_EQ(acceptor.Open(Net::SocketAddress(8888)), true);
}

TEST_F(ReactorPollTestSuite, open2) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), false);
}

TEST_F(ReactorPollTestSuite, open3) {
	MockAcceptor acceptor(reactor_);
	EXPECT_EQ(acceptor.Open(Net::SocketAddress(8888), 128, true), false);
}

TEST_F(ReactorPollTestSuite, close) {
	MockAcceptor acceptor(reactor_);
	acceptor.Close();
	acceptor_->Close();
}

TEST_F(ReactorPollTestSuite, accept_cb) {
	acceptor_->AcceptCallback(UV_ECANCELED);
}

TEST_F(ReactorPollTestSuite, accept_cb2) {
	acceptor_->AcceptCallback(0);
}

TEST_F(ReactorPollTestSuite, accept_cb3) {
	acceptor_->UseNullConnection(true);
	MockConnection connection;
	EXPECT_EQ(connector_->Connect(&connection, acceptor_address_), true);
	Poll();
}

TEST_F(ReactorPollTestSuite, accept_cb4) {
	acceptor_->UseExistConnection(true);
	MockConnection connection;
	EXPECT_EQ(connector_->Connect(&connection, acceptor_address_), true);
	Poll();
}

TEST_F(ReactorPollTestSuite, connect) {
	MockConnection connection;
	EXPECT_EQ(connector_->Connect(&connection, acceptor_address_), true);
	EXPECT_EQ(connector_->Connect(client_, acceptor_address_), false);
	EXPECT_EQ(client_->call_connect_failed_, 1);
}

TEST_F(ReactorPollTestSuite, connect_cb) {
	MockConnection connection;
	EXPECT_EQ(connector_->Connect(&connection, acceptor_address_), true);
	Poll();
}

TEST_F(ReactorPollTestSuite, connect_cb2) {
	MockConnection * connection = new MockConnection();
	EXPECT_EQ(connector_->Connect(connection, acceptor_address_), true);
	delete connection;
	Poll();
}

TEST_F(ReactorPollTestSuite, connect_cb3) {
	MockConnection connection;
	Net::SocketConnector * connector = new Net::SocketConnector(reactor_);
	EXPECT_EQ(connector->Connect(&connection, acceptor_address_), true);
	delete connector;
	Poll();
}

TEST_F(ReactorPollTestSuite, connect_cb4) {
	MockConnection connection;
	EXPECT_EQ(connector_->Connect(&connection, acceptor_address_), true);
	connection.GetSocket()->Close();
	Poll();
}

TEST_F(ReactorPollTestSuite, state) {
	MockConnection connection;
	EXPECT_EQ(connection_->GetConnectState(), Net::ConnectState::kConnected);
	EXPECT_EQ(connection.GetConnectState(), Net::ConnectState::kDisconnected);
	EXPECT_EQ(connector_->Connect(&connection, acceptor_address_), true);
	EXPECT_EQ(connection.GetConnectState(), Net::ConnectState::kConnecting);
}

TEST_F(ReactorPollTestSuite, socket) {
	Net::StreamSocket other_socket;
	MockConnection connection;
	Net::StreamSocket * socket = connection.GetSocket();
	EXPECT_NE(*socket, other_socket);
	connection.SetSocket(other_socket);
	socket = connection.GetSocket();
	EXPECT_EQ(*socket, other_socket);
}

TEST_F(ReactorPollTestSuite, recv) {
	i32 size;
	MockConnection connection;
	EXPECT_ANY_THROW(connection.GetRecvData(size));
	EXPECT_TRUE(client_->GetRecvData(size) == nullptr);
	EXPECT_EQ(size, 0);
	EXPECT_TRUE(connection_->GetRecvData(size) != nullptr);
	EXPECT_EQ(size, w_content_len_);
}

TEST_F(ReactorPollTestSuite, recv2) {
	MockConnection connection;
	EXPECT_ANY_THROW(client_->PopRecvData(w_content_len_));
	connection.PopRecvData(w_content_len_);
	connection_->PopRecvData(w_content_len_);
}

TEST_F(ReactorPollTestSuite, call) {
	MockConnection connection;
	connection.CallOnConnected();
	connection.CallOnConnected();
	connection.CallOnConnected();
	connection.CallOnConnectFailed(UV_ECANCELED);
	connection.CallOnConnectFailed(UV_EBADF);
	connection.CallOnConnectFailed(UV_EBUSY);
	EXPECT_EQ(connection.call_connected_, 1);
	EXPECT_EQ(connection.call_connect_failed_, 1);
	EXPECT_EQ(connection.call_disconnected_, 0);
}