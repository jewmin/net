#include "gtest/gtest.h"
#include "Reactor/EventHandler.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketAcceptor.h"
#include "Reactor/SocketConnector.h"
#include "Reactor/SocketConnection.h"

class MockSuccEventHandler : public Net::EventHandler {
public:
	MockSuccEventHandler(Net::EventReactor * reactor) : Net::EventHandler(reactor) {}
	virtual bool RegisterToReactor() { return true; }
	virtual bool UnRegisterFromReactor() { return true; }
};

class MockFailEventHandler : public Net::EventHandler {
public:
	MockFailEventHandler(Net::EventReactor * reactor) : Net::EventHandler(reactor) {}
	virtual bool RegisterToReactor() { return false; }
	virtual bool UnRegisterFromReactor() { return false; }
};

TEST(ReactorTest, reactor_ctor) {
	Net::EventReactor reactor;
	EXPECT_EQ(reactor.Poll(), false);
	EXPECT_FALSE(reactor.GetUvLoop() == nullptr);
}

TEST(ReactorTest, handler) {
	Net::EventReactor reactor;
	MockSuccEventHandler * h1 = new MockSuccEventHandler(&reactor);
	MockFailEventHandler * h2 = new MockFailEventHandler(nullptr);
	EXPECT_TRUE(h1->GetReactor() == &reactor);
	EXPECT_TRUE(h2->GetReactor() == nullptr);
	h1->SetReactor(nullptr);
	h2->SetReactor(&reactor);
	EXPECT_TRUE(h1->GetReactor() == nullptr);
	EXPECT_TRUE(h2->GetReactor() == &reactor);
	EXPECT_EQ(h1->ReferenceCount(), 1);
	EXPECT_EQ(h2->ReferenceCount(), 1);
	h1->Release();
	h2->Release();
}

TEST(ReactorTest, reactor_add) {
	Net::EventReactor reactor;
	MockSuccEventHandler * h1 = new MockSuccEventHandler(&reactor);
	MockFailEventHandler * h2 = new MockFailEventHandler(&reactor);
	Net::SocketConnection * h3 = new Net::SocketConnection(50, 50);
	h3->SetReactor(&reactor);
	EXPECT_EQ(h1->ReferenceCount(), 1);
	EXPECT_EQ(h2->ReferenceCount(), 1);
	EXPECT_EQ(h3->ReferenceCount(), 1);
	EXPECT_EQ(reactor.AddEventHandler(h1), true);
	EXPECT_EQ(reactor.AddEventHandler(h2), false);
	EXPECT_EQ(reactor.AddEventHandler(h3), false);
	EXPECT_EQ(h1->ReferenceCount(), 2);
	EXPECT_EQ(h2->ReferenceCount(), 1);
	EXPECT_EQ(h3->ReferenceCount(), 1);
	h1->Release();
	h2->Release();
	h3->Release();
}

TEST(ReactorTest, reactor_remove) {
	Net::EventReactor reactor;
	MockSuccEventHandler * h1 = new MockSuccEventHandler(&reactor);
	MockFailEventHandler * h2 = new MockFailEventHandler(&reactor);
	Net::SocketConnection * h3 = new Net::SocketConnection(50, 50);
	h3->SetReactor(&reactor);
	EXPECT_EQ(h1->ReferenceCount(), 1);
	EXPECT_EQ(h2->ReferenceCount(), 1);
	EXPECT_EQ(h3->ReferenceCount(), 1);
	EXPECT_EQ(reactor.AddEventHandler(h1), true);
	EXPECT_EQ(reactor.AddEventHandler(h2), false);
	EXPECT_EQ(reactor.AddEventHandler(h3), false);
	EXPECT_EQ(h1->ReferenceCount(), 2);
	EXPECT_EQ(h2->ReferenceCount(), 1);
	EXPECT_EQ(h3->ReferenceCount(), 1);
	EXPECT_EQ(reactor.RemoveEventHandler(h1), true);
	EXPECT_EQ(reactor.RemoveEventHandler(h2), false);
	EXPECT_EQ(reactor.RemoveEventHandler(h3), false);
	EXPECT_EQ(h1->ReferenceCount(), 1);
	EXPECT_EQ(h2->ReferenceCount(), 1);
	EXPECT_EQ(h3->ReferenceCount(), 1);
	h1->Release();
	h2->Release();
	h3->Release();
}

class ReactorTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		succ_handler_ = new MockSuccEventHandler(nullptr);
		fail_handler_ = new MockFailEventHandler(&reactor_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		succ_handler_->Release();
		fail_handler_->Release();
		reactor_.ClearEventHandlers();
	}

	Net::EventReactor * GetReactor() {
		return &reactor_;
	}

	uv_loop_t * GetUvLoop() const {
		return reactor_.GetUvLoop();
	}

	static void close_cb(uv_handle_t* handle) {
		jc_free(handle);
	}

	static void timer_cb(uv_timer_t * handle) {
		uv_close(reinterpret_cast<uv_handle_t *>(handle), close_cb);
	}

	MockSuccEventHandler * succ_handler_;
	MockFailEventHandler * fail_handler_;
	Net::EventReactor reactor_;
};

TEST(ReactorTest, reactor_timer) {
	Net::EventReactor reactor;
	uv_timer_t * timer = static_cast<uv_timer_t *>(jc_malloc(sizeof(*timer)));
	uv_timer_init(reactor.GetUvLoop(), timer);
	uv_timer_start(timer, ReactorTestSuite::timer_cb, 10, 0);
	EXPECT_EQ(reactor.Poll(), true);
	EXPECT_EQ(reactor.Poll(UV_RUN_DEFAULT), false);
}

TEST(ReactorTest, reactor_dtor) {
	Net::EventReactor reactor;
	uv_timer_t * timer = static_cast<uv_timer_t *>(jc_malloc(sizeof(*timer)));
	uv_timer_init(reactor.GetUvLoop(), timer);
	uv_timer_start(timer, ReactorTestSuite::timer_cb, 10, 0);
}

class MockConnection : public Net::SocketConnection {
public:
	MockConnection() : Net::SocketConnection(60, 50), call_connected_(0), call_disconnected_(0), call_recv_(0), call_sent_(0), call_error_(0) {}
	virtual void OnConnected() {
		Net::SocketConnection::OnConnected();
		call_connected_++;
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
	}

	i32 call_connected_;
	i32 call_disconnected_;
	i32 call_recv_;
	i32 call_sent_;
	i32 call_error_;
};

class MockNullAcceptor : public Net::SocketAcceptor {
public:
	MockNullAcceptor(Net::EventReactor * reactor) : Net::SocketAcceptor(reactor) {}
	virtual ~MockNullAcceptor() {}
	virtual Net::SocketConnection * CreateConnection() { return nullptr; }
	virtual void DestroyConnection(Net::SocketConnection * connection) {}
	virtual void AcceptCallback(i32 status) {
		Net::SocketAcceptor::AcceptCallback(status);
	}
};

TEST_F(ReactorTestSuite, accept_open) {
	Net::SocketAddress ipv6_address("::", 9999), ipv4_address(10000);
	Net::SocketAcceptor * acceptor = new MockNullAcceptor(GetReactor());
	EXPECT_EQ(acceptor->ReferenceCount(), 1);

	acceptor->Close();
	EXPECT_EQ(acceptor->Open(ipv6_address), true);
	EXPECT_EQ(acceptor->GetListenAddress(), ipv6_address);
	EXPECT_EQ(acceptor->ReferenceCount(), 2);

	EXPECT_EQ(acceptor->Open(ipv6_address), false);

	acceptor->Close();
	EXPECT_EQ(acceptor->ReferenceCount(), 1);

	EXPECT_EQ(acceptor->Open(ipv4_address, 128, true), false);

	EXPECT_EQ(acceptor->Open(ipv4_address), true);
	EXPECT_EQ(acceptor->GetListenAddress(), ipv4_address);
	EXPECT_EQ(acceptor->ReferenceCount(), 2);
	acceptor->Release();
}

TEST_F(ReactorTestSuite, accept_cb_err) {
	MockNullAcceptor * acceptor = new MockNullAcceptor(GetReactor());
	acceptor->AcceptCallback(UV_UNKNOWN);
	acceptor->AcceptCallback(0);
	acceptor->Release();
}

class AcceptorTestSuite : public ReactorTestSuite {
public:
	AcceptorTestSuite() : port_(10001) {
		std::memcpy(w_content_, "hello world", std::strlen("hello world"));
		w_content_len_ = static_cast<i32>(std::strlen("hello world"));
		w_content_[w_content_len_] = 0;
	}

	// Sets up the test fixture.
	virtual void SetUp() {
		ReactorTestSuite::SetUp();
		port_++;
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		ReactorTestSuite::TearDown();
	}

	void Poll(i32 count = 30) {
		while (count-- > 0) { GetReactor()->Poll(); }
	}

	i8 w_content_[12];
	i32 w_content_len_;
	u16 port_;
};

TEST_F(AcceptorTestSuite, accept_cb_null) {
	Net::StreamSocket socket;
	Net::SocketAcceptor * acceptor = new MockNullAcceptor(GetReactor());
	EXPECT_EQ(acceptor->ReferenceCount(), 1);
	EXPECT_EQ(acceptor->Open(Net::SocketAddress(port_)), true);
	EXPECT_EQ(acceptor->ReferenceCount(), 2);
	socket.Open(GetReactor()->GetUvLoop());
	EXPECT_EQ(socket.Connect(Net::SocketAddress("127.0.0.1", port_)), 0);
	Poll();
	EXPECT_EQ(acceptor->ReferenceCount(), 2);
	acceptor->Release();
}

class MockDupAcceptor : public MockNullAcceptor {
public:
	MockDupAcceptor(Net::EventReactor * reactor) : MockNullAcceptor(reactor), connection_(new MockConnection()) {}
	virtual ~MockDupAcceptor() { connection_->Release(); }
	virtual Net::SocketConnection * CreateConnection() { return connection_; }
	virtual void DestroyConnection(Net::SocketConnection * connection) {}
	MockConnection * connection_;
};

TEST_F(AcceptorTestSuite, accept_cb_repeat) {
	Net::StreamSocket socket, socket_dup;
	MockDupAcceptor * acceptor = new MockDupAcceptor(GetReactor());
	EXPECT_EQ(acceptor->ReferenceCount(), 1);
	EXPECT_EQ(acceptor->connection_->ReferenceCount(), 1);
	EXPECT_EQ(acceptor->connection_->call_connected_, 0);
	EXPECT_EQ(acceptor->Open(Net::SocketAddress(port_)), true);
	socket.Open(GetReactor()->GetUvLoop());
	EXPECT_EQ(socket.Connect(Net::SocketAddress("127.0.0.1", port_)), 0);
	Poll();
	EXPECT_EQ(acceptor->ReferenceCount(), 2);
	EXPECT_EQ(acceptor->connection_->ReferenceCount(), 2);
	EXPECT_EQ(acceptor->connection_->call_connected_, 1);
	socket_dup.Open(GetReactor()->GetUvLoop());
	EXPECT_EQ(socket_dup.Connect(Net::SocketAddress("127.0.0.1", port_)), 0);
	Poll();
	EXPECT_EQ(acceptor->ReferenceCount(), 2);
	EXPECT_EQ(acceptor->connection_->ReferenceCount(), 2);
	EXPECT_EQ(acceptor->connection_->call_connected_, 1);
	acceptor->Release();
}

class MockAcceptor : public MockNullAcceptor {
public:
	MockAcceptor(Net::EventReactor * reactor) : MockNullAcceptor(reactor) {}
	virtual ~MockAcceptor() {
		for (auto & it : connection_list_) {
			it->Release();
		}
	}
	virtual Net::SocketConnection * CreateConnection() {
		Net::SocketConnection * connection = new MockConnection();
		connection_list_.push_back(connection);
		return connection;
	}
	virtual void DestroyConnection(Net::SocketConnection * connection) {
		connection_list_.remove(connection);
		connection->Release();
	}
	void WriteAll(i8 * data, i32 len) {
		for (auto & it : connection_list_) {
			EXPECT_EQ(it->Write(data, len), len);
		}
	}
	void ShutdownAll(bool now) {
		for (auto & it : connection_list_) {
			it->Shutdown(now);
		}
	}
	std::list<Net::SocketConnection *> connection_list_;
};

class ConnectorTestSuite : public AcceptorTestSuite {
public:
	ConnectorTestSuite() { port_ += 10; }

	// Sets up the test fixture.
	virtual void SetUp() {
		AcceptorTestSuite::SetUp();
		acceptor_ = new MockAcceptor(GetReactor());
		EXPECT_EQ(acceptor_->Open(Net::SocketAddress("::", port_)), true);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		acceptor_->Release();
		AcceptorTestSuite::TearDown();
	}

	MockAcceptor * acceptor_;
};

class MockNullConnector : public Net::SocketConnector {
public:
	MockNullConnector(Net::EventReactor * reactor) : Net::SocketConnector(reactor), connect_failed_(0) {}
	virtual Net::SocketConnection * CreateConnection() override { return nullptr; }
	virtual void DestroyConnection(Net::SocketConnection * connection) override {}
	virtual void OnConnectFailed() override { connect_failed_++; }
	i32 connect_failed_;
};

TEST_F(ConnectorTestSuite, close_fail) {
	MockNullConnector * connector = new MockNullConnector(GetReactor());
	EXPECT_EQ(connector->ReferenceCount(), 1);
	connector->Close();
	connector->Release();
}

TEST_F(ConnectorTestSuite, connect_fail) {
	MockNullConnector * connector = new MockNullConnector(GetReactor());
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->Connect(Net::SocketAddress(0)), false);
	connector->Release();
}

TEST_F(ConnectorTestSuite, connect_succ) {
	MockNullConnector * connector = new MockNullConnector(GetReactor());
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
	EXPECT_EQ(connector->ReferenceCount(), 2);
	EXPECT_EQ(connector->Connect(Net::SocketAddress("::", port_)), false);
	connector->Release();
}

TEST_F(ConnectorTestSuite, close_succ) {
	MockNullConnector * connector = new MockNullConnector(GetReactor());
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
	EXPECT_EQ(connector->ReferenceCount(), 2);
	connector->Close();
	Poll();
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->connect_failed_, 1);
	connector->Release();
}

TEST_F(ConnectorTestSuite, connect_cb_null) {
	MockNullConnector * connector = new MockNullConnector(GetReactor());
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
	EXPECT_EQ(connector->ReferenceCount(), 2);
	Poll();
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->connect_failed_, 1);
	connector->Release();
}

class MockDupConnector : public MockNullConnector {
public:
	MockDupConnector(Net::EventReactor * reactor) : MockNullConnector(reactor), connection_(new MockConnection()) {}
	virtual ~MockDupConnector() { connection_->Release(); }
	virtual Net::SocketConnection * CreateConnection() { return connection_; }
	virtual void DestroyConnection(Net::SocketConnection * connection) {}
	MockConnection * connection_;
};

TEST_F(ConnectorTestSuite, connect_cb_repeat) {
	MockDupConnector * connector = new MockDupConnector(GetReactor());
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
	EXPECT_EQ(connector->ReferenceCount(), 2);
	Poll();
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->connect_failed_, 0);
	EXPECT_EQ(connector->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
	EXPECT_EQ(connector->ReferenceCount(), 2);
	Poll();
	EXPECT_EQ(connector->ReferenceCount(), 1);
	EXPECT_EQ(connector->connect_failed_, 1);
	connector->Release();
}

class MockWriteConnection : public MockConnection {
public:
	virtual void OnNewDataReceived() override {
		MockConnection::OnNewDataReceived();
		i8 buff[250] = {0};
		EXPECT_GT(Read(buff, sizeof(250)), 0);
	}
};

class MockConnector : public MockNullConnector {
public:
	MockConnector(Net::EventReactor * reactor) : MockNullConnector(reactor), connection_(nullptr) {}
	virtual ~MockConnector() {
		if (connection_) {
			connection_->Release();
		}
	}
	virtual Net::SocketConnection * CreateConnection() {
		connection_ = new MockWriteConnection();
		return connection_;
	}
	virtual void DestroyConnection(Net::SocketConnection * connection) {
		connection_->Release();
		connection_ = nullptr;
	}
	MockWriteConnection * connection_;
};

class ConnectionTestSuite : public ConnectorTestSuite {
public:
	ConnectionTestSuite() { port_ += 10; }

	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectorTestSuite::SetUp();
		connector_ = new MockConnector(GetReactor());
		EXPECT_EQ(connector_->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
		Poll();
		EXPECT_EQ(connector_->connection_->call_connected_, 1);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		connector_->Release();
		ConnectorTestSuite::TearDown();
	}

	MockConnector * connector_;
};

TEST_F(ConnectionTestSuite, disconnected_conn) {
	MockConnection connection;
	EXPECT_EQ(connection.GetConnectState(), Net::ConnectState::kDisconnected);
	Net::StreamSocket socket;
	EXPECT_NE(*connection.GetSocket(), socket);
	connection.SetSocket(socket);
	EXPECT_EQ(*connection.GetSocket(), socket);
	EXPECT_TRUE(connection.GetRecvData() == nullptr);
	EXPECT_EQ(connection.GetRecvDataSize(), 0);
	connection.PopRecvData(10);
	EXPECT_EQ(connection.Write(w_content_, w_content_len_), UV_ENOTCONN);
	i8 buff[256] = {0};
	EXPECT_EQ(connection.Read(buff, sizeof(buff)), UV_ENOTCONN);
	connection.Shutdown(false);
}

TEST_F(ConnectionTestSuite, connected_conn) {
	EXPECT_EQ(connector_->connection_->GetConnectState(), Net::ConnectState::kConnected);
	EXPECT_TRUE(connector_->connection_->GetRecvData() == nullptr);
	EXPECT_EQ(connector_->connection_->GetRecvDataSize(), 0);
	connector_->connection_->PopRecvData(10);
	connector_->connection_->Shutdown(true);
	Poll();
}

TEST_F(ConnectionTestSuite, write_succ) {
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), UV_ENOBUFS);
	connector_->connection_->Shutdown(false);
	Poll();
	EXPECT_EQ(connector_->connection_->call_error_, 0);
	EXPECT_EQ(connector_->connection_->call_sent_, 5);
}

TEST_F(ConnectionTestSuite, write_close) {
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	connector_->connection_->Shutdown(true);
	Poll();
	EXPECT_EQ(connector_->connection_->call_error_, 0);
	EXPECT_EQ(connector_->connection_->call_sent_, 0);
}

TEST_F(ConnectionTestSuite, read) {
	acceptor_->WriteAll(w_content_, w_content_len_);
	Poll();
	EXPECT_EQ(connector_->connection_->call_error_, 0);
	EXPECT_EQ(connector_->connection_->call_recv_, 1);
}

class TwoReactorTestSuite : public ConnectorTestSuite {
public:
	TwoReactorTestSuite() { port_ += 20; }

	// Sets up the test fixture.
	virtual void SetUp() {
		ConnectorTestSuite::SetUp();
		connector_ = new MockConnector(&connect_reactor_);
		EXPECT_EQ(connector_->Connect(Net::SocketAddress("127.0.0.1", port_)), true);
		Poll();
		ConnectPoll();
		EXPECT_EQ(connector_->connection_->call_connected_, 1);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		connector_->Release();
		connect_reactor_.ClearEventHandlers();
		ConnectorTestSuite::TearDown();
	}

	void ConnectPoll(i32 count = 30) {
		while (count-- > 0) { connect_reactor_.Poll(); }
	}

	MockConnector * connector_;
	Net::EventReactor connect_reactor_;
};

TEST_F(TwoReactorTestSuite, eof) {
	EXPECT_EQ(connector_->connection_->GetSocket()->ShutdownRead(), 0);
	EXPECT_EQ(connector_->connection_->Write(w_content_, w_content_len_), w_content_len_);
	acceptor_->WriteAll(w_content_, w_content_len_);
	connector_->connection_->Shutdown(false);
	Poll();
	ConnectPoll();
}