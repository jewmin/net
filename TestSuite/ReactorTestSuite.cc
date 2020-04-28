#include "gtest/gtest.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketConnector.h"
#include "Reactor/SocketAcceptor.h"
#include "Sockets/StreamSocketImpl.h"

static i32 ReactorTestSuite_Call_Connected = 0;
static i32 ReactorTestSuite_Call_ConnectFailed = 0;
static i32 ReactorTestSuite_Call_Disconnected = 0;
static i32 ReactorTestSuite_Call_NewDataReceived = 0;
static i32 ReactorTestSuite_Call_SomeDataSent = 0;
static i32 ReactorTestSuite_Call_Error = 0;

class ReactorTestSuiteMockEventHandler : public Net::EventHandler {
public:
	ReactorTestSuiteMockEventHandler(Net::EventReactor * reactor) : Net::EventHandler(reactor) {}
	virtual ~ReactorTestSuiteMockEventHandler() {}
	virtual bool RegisterToReactor() { return true; }
	virtual bool UnRegisterFromReactor() { return true; }
};

class ReactorTestSuiteMockEventHandler2 : public Net::EventHandler {
public:
	ReactorTestSuiteMockEventHandler2(Net::EventReactor * reactor) : Net::EventHandler(reactor) {}
	virtual ~ReactorTestSuiteMockEventHandler2() {}
	virtual bool RegisterToReactor() { Connect(); return true; }
	virtual bool UnRegisterFromReactor() { delete this; return true; }
	void Connect() {
		socket_.Open(GetReactor()->GetUvLoop());
		socket_.Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789));
	}
	Net::StreamSocket socket_;
};

class ReactorTestSuiteMockConnection : public Net::SocketConnection {
public:
	ReactorTestSuiteMockConnection(i32 out_size = 1024, i32 in_size = 1024) : Net::SocketConnection(out_size, in_size) {}
	virtual ~ReactorTestSuiteMockConnection() {}
	virtual void OnConnected() { Net::SocketConnection::OnConnected(); ReactorTestSuite_Call_Connected++; }
	virtual void OnConnectFailed(i32 reason) { Net::SocketConnection::OnConnectFailed(reason); ReactorTestSuite_Call_ConnectFailed++; }
	virtual void OnDisconnected(bool is_remote) { Net::SocketConnection::OnDisconnected(is_remote); ReactorTestSuite_Call_Disconnected++; }
	virtual void OnNewDataReceived() { Net::SocketConnection::OnNewDataReceived(); ReactorTestSuite_Call_NewDataReceived++; }
	virtual void OnSomeDataSent() { Net::SocketConnection::OnSomeDataSent(); ReactorTestSuite_Call_SomeDataSent++; }
	virtual void OnError(i32 reason) { std::printf("OnError: %s\n", uv_strerror(reason)); Net::SocketConnection::OnError(reason); ReactorTestSuite_Call_Error++; }
};

class ReactorTestSuiteMockStreamSocketImpl : public Net::StreamSocketImpl {
public:
	virtual i32 Connect(const Net::SocketAddress & address, void * arg = nullptr) {
		return UV_ENOTSUP;
	}
};

class ReactorTestSuiteMockStreamSocket : public Net::StreamSocket {
public:
	ReactorTestSuiteMockStreamSocket() : Net::StreamSocket(new ReactorTestSuiteMockStreamSocketImpl()) {}
	virtual ~ReactorTestSuiteMockStreamSocket() {}
};

class ReactorTestSuiteMockConnection2 : public ReactorTestSuiteMockConnection {
public:
	virtual Net::StreamSocket * GetSocket() { return &socket_; }
	ReactorTestSuiteMockStreamSocket socket_;
};

class ReactorTestSuiteMockConnection3 : public ReactorTestSuiteMockConnection {
public:
	virtual void OnConnected() {
		ReactorTestSuiteMockConnection::OnConnected();
		Write("123", 3);
	}
};

class ReactorTestSuiteMockConnection4 : public ReactorTestSuiteMockConnection {
public:
	virtual void TestError(i32 reason) {
		Error(reason);
	}
};

class ReactorTestSuiteMockAcceptor : public Net::SocketAcceptor {
public:
	ReactorTestSuiteMockAcceptor(Net::EventReactor * reactor) : Net::SocketAcceptor(reactor), connection_(nullptr) {}
	virtual ~ReactorTestSuiteMockAcceptor() {}
	virtual void Destroy() {
		if (connection_) {
			connection_->Destroy();
			connection_ = nullptr;
		}
		Net::SocketAcceptor::Destroy();
	}
	virtual Net::SocketConnection * CreateConnection() {
		if (connection_) {
			connection_->Destroy();
		}
		connection_ = new ReactorTestSuiteMockConnection();
		return connection_;
	}
	Net::SocketConnection * connection_;
};

class ReactorTestSuiteMockAcceptor2 : public ReactorTestSuiteMockAcceptor {
public:
	ReactorTestSuiteMockAcceptor2(Net::EventReactor * reactor) : ReactorTestSuiteMockAcceptor(reactor) {}
	virtual ~ReactorTestSuiteMockAcceptor2() {}
	void TestAcceptCallback(i32 status) { AcceptCallback(status); }
	virtual Net::SocketConnection * CreateConnection() { return nullptr; }
};

class ReactorTestSuiteMockAcceptor3 : public ReactorTestSuiteMockAcceptor2 {
public:
	ReactorTestSuiteMockAcceptor3(Net::EventReactor * reactor) : ReactorTestSuiteMockAcceptor2(reactor) {}
	virtual ~ReactorTestSuiteMockAcceptor3() {}
	virtual Net::SocketConnection * CreateConnection() {
		if (connection_) {
			connection_->Destroy();
		}
		connection_ = new ReactorTestSuiteMockConnection();
		connection_->SetConnectState(Net::ConnectState::kDisconnecting);
		return connection_;
	}
};

class ReactorTestSuiteMockAcceptor4 : public ReactorTestSuiteMockAcceptor3 {
public:
	ReactorTestSuiteMockAcceptor4(Net::EventReactor * reactor) : ReactorTestSuiteMockAcceptor3(reactor) {}
	virtual ~ReactorTestSuiteMockAcceptor4() {}
	virtual Net::SocketConnection * CreateConnection() {
		if (connection_) {
			connection_->Destroy();
		}
		connection_ = new ReactorTestSuiteMockConnection3();
		return connection_;
	}
};

class ReactorTestSuiteMock : public Net::UvData {
public:
	ReactorTestSuiteMock(Net::EventReactor * reactor, Net::SocketConnection * connection) : reactor_(reactor), connection_(connection) {}
	virtual ~ReactorTestSuiteMock() {}
	virtual void ConnectCallback(i32 status, void * arg) {
		connection_->GetSocket()->SetUvData(connection_);
		connection_->SetReactor(reactor_);
		reactor_->AddEventHandler(connection_);
		try {
			connection_->SetConnectState(Net::ConnectState::kConnecting);
			connection_->GetSocket()->ShutdownRead();
			reactor_->AddEventHandler(connection_);
		} catch (std::exception & e) {
			std::printf("ReactorTestSuiteMock - Catch: %s\n", e.what());
		}
		EXPECT_EQ(connection_->Write("123", 3), UV_ENOTCONN);
		connection_->SetConnectState(Net::ConnectState::kConnected);
		EXPECT_EQ(connection_->Write("123", 3), 3);
		connection_->Shutdown(false);
	}
	Net::EventReactor * reactor_;
	Net::SocketConnection * connection_;
};

class ReactorTestSuiteTest : public testing::Test {
protected:
	virtual void SetUp() {
		address_ = Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789);
		ReactorTestSuite_Call_Connected = 0;
		ReactorTestSuite_Call_ConnectFailed = 0;
		ReactorTestSuite_Call_Disconnected = 0;
		ReactorTestSuite_Call_NewDataReceived = 0;
		ReactorTestSuite_Call_SomeDataSent = 0;
		ReactorTestSuite_Call_Error = 0;
	}
	virtual void TearDown() {
	}
	void Run(i32 count) {
		while (count-- > 0) {
			reactor_.Poll();
		}
	}

public:
	Net::EventReactor reactor_;
	Net::SocketAddress address_;
};

TEST_F(ReactorTestSuiteTest, EventHandler) {
	ReactorTestSuiteMockEventHandler * handler = new ReactorTestSuiteMockEventHandler(&reactor_);
	ReactorTestSuiteMockEventHandler2 * handler2 = new ReactorTestSuiteMockEventHandler2(&reactor_);
	EXPECT_EQ(reactor_.AddEventHandler(handler), true);
	EXPECT_EQ(reactor_.AddEventHandler(handler2), true);
	EXPECT_EQ(reactor_.RemoveEventHandler(handler), true);
	EXPECT_TRUE(handler->GetReactor() == &reactor_);
	handler->SetReactor(nullptr);
	handler->Destroy();
}

TEST_F(ReactorTestSuiteTest, Reactor) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	acceptor->Open(address_);
	EXPECT_EQ(reactor_.AddEventHandler(connector), false);
	EXPECT_EQ(reactor_.RemoveEventHandler(connector), false);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	connector->Destroy();
	acceptor->Destroy();
	connection->Destroy();
	reactor_.Poll(UV_RUN_DEFAULT);
	EXPECT_EQ(ReactorTestSuite_Call_Connected, 2);
	EXPECT_EQ(ReactorTestSuite_Call_ConnectFailed, 0);
	EXPECT_EQ(ReactorTestSuite_Call_Disconnected, 2);
	EXPECT_EQ(ReactorTestSuite_Call_NewDataReceived, 0);
	EXPECT_EQ(ReactorTestSuite_Call_SomeDataSent, 0);
	EXPECT_EQ(ReactorTestSuite_Call_Error, 0);
}

TEST_F(ReactorTestSuiteTest, acceptor) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	EXPECT_EQ(acceptor->Open(address_, 128, true), false);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(acceptor->Open(address_), false);
	acceptor->Destroy();
}

TEST_F(ReactorTestSuiteTest, acceptor_cb) {
	ReactorTestSuiteMockAcceptor2 * acceptor = new ReactorTestSuiteMockAcceptor2(&reactor_);
	acceptor->TestAcceptCallback(UV_ECANCELED);
	acceptor->Destroy();
}

TEST_F(ReactorTestSuiteTest, acceptor_cb2) {
	ReactorTestSuiteMockAcceptor2 * acceptor = new ReactorTestSuiteMockAcceptor2(&reactor_);
	acceptor->TestAcceptCallback(0);
	acceptor->Destroy();
}

TEST_F(ReactorTestSuiteTest, acceptor_cb3) {
	ReactorTestSuiteMockAcceptor2 * acceptor = new ReactorTestSuiteMockAcceptor2(&reactor_);
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	connector->Destroy();
	connection->Destroy();
	acceptor->Destroy();
	EXPECT_EQ(ReactorTestSuite_Call_Connected, 1);
	EXPECT_EQ(ReactorTestSuite_Call_Disconnected, 1);
}

TEST_F(ReactorTestSuiteTest, acceptor_cb4) {
	ReactorTestSuiteMockAcceptor3 * acceptor = new ReactorTestSuiteMockAcceptor3(&reactor_);
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	connector->Destroy();
	connection->Destroy();
	acceptor->Destroy();
	EXPECT_EQ(ReactorTestSuite_Call_Connected, 1);
	EXPECT_EQ(ReactorTestSuite_Call_ConnectFailed, 1);
	EXPECT_EQ(ReactorTestSuite_Call_Disconnected, 1);
}

TEST_F(ReactorTestSuiteTest, connector) {
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection2();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(connector->Connect(connection, address_), false);
	EXPECT_EQ(connector->Connect(connection, address_), false);
	connector->Destroy();
	connection->Destroy();
	EXPECT_EQ(ReactorTestSuite_Call_ConnectFailed, 1);
}

TEST_F(ReactorTestSuiteTest, connector_cb) {
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	connection->Shutdown(false);
	Run(3);
	connector->Destroy();
	connection->Destroy();
	EXPECT_EQ(ReactorTestSuite_Call_ConnectFailed, 1);
}

TEST_F(ReactorTestSuiteTest, connector_cb2) {
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	connection->GetSocket()->Close();
	Run(3);
	connector->Destroy();
	connection->Destroy();
	EXPECT_EQ(ReactorTestSuite_Call_ConnectFailed, 1);
}

TEST_F(ReactorTestSuiteTest, connector_cb3) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	connection->SetConnectState(Net::ConnectState::kConnected);
	Run(3);
	connection->SetConnectState(Net::ConnectState::kDisconnected);
	connector->Destroy();
	connection->Destroy();
	acceptor->Destroy();
	EXPECT_EQ(ReactorTestSuite_Call_ConnectFailed, 1);
	EXPECT_EQ(ReactorTestSuite_Call_Connected, 1);
	EXPECT_EQ(ReactorTestSuite_Call_Disconnected, 1);
}

TEST_F(ReactorTestSuiteTest, connection) {
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	EXPECT_EQ(reactor_.AddEventHandler(connection), false);
	EXPECT_EQ(reactor_.RemoveEventHandler(connection), false);
	connection->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection1) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	Net::SocketConnection * connection = new ReactorTestSuiteMockConnection();
	EXPECT_EQ(acceptor->Open(address_), true);
	connection->GetSocket()->Open(reactor_.GetUvLoop());
	connection->GetSocket()->Connect(address_);
	connection->GetSocket()->SetUvData(new ReactorTestSuiteMock(&reactor_, connection));
	Run(3);
	connection->Destroy();
	acceptor->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection2) {
	char buf[128] = { 0 };
	i32 len = 0;
	Net::SocketConnection * connection = new Net::SocketConnection(128, 128);
	try {
		connection->Read(buf, sizeof(buf));
	} catch (std::exception &) {}
	try {
		connection->GetRecvData(len);
	} catch (std::exception &) {}
	try {
		connection->PopRecvData(len);
	} catch (std::exception &) {}
	connection->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection3) {
	char buf[128] = { 0 };
	i8 * block = nullptr;
	i32 len = 0;
	Net::SocketConnection * connection = new Net::SocketConnection(128, 128);
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor4(&reactor_);
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	block = connection->GetRecvData(len);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(len, 3);
	connection->PopRecvData(1);
	len = connection->Read(buf, sizeof(buf));
	buf[len] = 0;
	EXPECT_EQ(len, 2);
	EXPECT_STREQ(buf, "23");
	acceptor->Destroy();
	connector->Destroy();
	connection->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection4) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	ReactorTestSuiteMockConnection4 * connection = new ReactorTestSuiteMockConnection4();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	connection->TestError(UV_UNKNOWN);
	connection->Destroy();
	acceptor->Destroy();
	connector->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection5) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	ReactorTestSuiteMockConnection4 * connection = new ReactorTestSuiteMockConnection4();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	connection->TestError(UV_EOF);
	connection->Destroy();
	acceptor->Destroy();
	connector->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection6) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	ReactorTestSuiteMockConnection4 * connection = new ReactorTestSuiteMockConnection4();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	EXPECT_EQ(connection->Write("123", 3), 3);
	connection->Shutdown(false);
	connection->TestError(UV_EOF);
	connection->Destroy();
	acceptor->Destroy();
	connector->Destroy();
}

TEST_F(ReactorTestSuiteTest, connection7) {
	Net::SocketAcceptor * acceptor = new ReactorTestSuiteMockAcceptor(&reactor_);
	ReactorTestSuiteMockConnection4 * connection = new ReactorTestSuiteMockConnection4();
	Net::SocketConnector * connector = new Net::SocketConnector(&reactor_);
	EXPECT_EQ(acceptor->Open(address_), true);
	EXPECT_EQ(connector->Connect(connection, address_), true);
	Run(3);
	EXPECT_EQ(connection->Write("123", 3), 3);
	connection->TestError(UV_EOF);
	connection->Destroy();
	acceptor->Destroy();
	connector->Destroy();
}