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

TEST_F(ReactorTestSuite, reactor) {
	EXPECT_EQ(reactor_->Poll(), false);
}

TEST_F(ReactorTestSuite, reactor2) {
	uv_timer_t * timer = static_cast<uv_timer_t *>(jc_malloc(sizeof(*timer)));
	uv_timer_init(reactor_->GetUvLoop(), timer);
	uv_timer_start(timer, timer_cb, 10, 0);
}

class MockAcceptor : public Net::SocketAcceptor {
public:
	MockAcceptor(Net::EventReactor * reactor)
		: Net::SocketAcceptor(reactor), connection_(new Net::SocketConnection(1024, 1024)) {}
	virtual ~MockAcceptor() { connection_->Destroy(); }
	virtual Net::SocketConnection * CreateConnection() {
		return connection_;
	}
	Net::SocketConnection * connection_;
};

class AcceptorTestSuite : public ReactorTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ReactorTestSuite::SetUp();
		acceptor_ = new MockAcceptor(reactor_);
		acceptor_address_ = Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		acceptor_->Destroy();
		ReactorTestSuite::TearDown();
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

class MockConnection : public Net::SocketConnection {
public:
	MockConnection() : Net::SocketConnection(1024, 1024) {}
	virtual Net::StreamSocket * GetSocket() { return &socket_; }
	MockStreamSocket socket_;
};

class ConnectorTestSuite : public AcceptorTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		AcceptorTestSuite::SetUp();
		connector_ = new Net::SocketConnector(reactor_);
		connection_ = new MockConnection();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		connection_->Destroy();
		connector_->Destroy();
		AcceptorTestSuite::TearDown();
	}

	MockConnection * connection_;
	Net::SocketConnector * connector_;
};

TEST_F(ConnectorTestSuite, handler) {
	EXPECT_EQ(reactor_->AddEventHandler(connector_), false);
	EXPECT_EQ(reactor_->RemoveEventHandler(connector_), false);
}

TEST_F(ConnectorTestSuite, connect) {
	EXPECT_EQ(connector_->Connect(connection_, acceptor_address_), true);
}

TEST_F(ConnectorTestSuite, connect2) {
	Net::StreamSocket * socket = connection_->GetSocket();
	socket->Open(reactor_->GetUvLoop());
#ifdef _WIN32
	dynamic_cast<MockStreamSocketImpl *>(socket->Impl())->delayed_error(UV_EALREADY);
#else
	EXPECT_EQ(connector_->Connect(connection_, acceptor_address_), true);
#endif
	EXPECT_EQ(connector_->Connect(connection_, acceptor_address_), false);
}

class ConnectionTestSuite : public ConnectorTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectorTestSuite::SetUp();
		succ_connection_ = new Net::SocketConnection(1024, 1024);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		succ_connection_->Destroy();
		ConnectorTestSuite::TearDown();
	}

	void Poll(i32 count = 30) {
		while (count-- > 0) {
			reactor_->Poll();
		}
	}

	Net::SocketConnection * succ_connection_;
};

TEST_F(ConnectionTestSuite, accept_succ) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
	EXPECT_EQ(connector_->Connect(connection_, acceptor_address_), true);
	Poll();
}

TEST_F(ConnectionTestSuite, accept_succ2) {
	EXPECT_EQ(acceptor_->Open(acceptor_address_), true);
	EXPECT_EQ(connector_->Connect(succ_connection_, acceptor_address_), true);
	Poll();
}