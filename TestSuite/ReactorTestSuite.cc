#include "gtest/gtest.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketConnector.h"
#include "Reactor/SocketAcceptor.h"

using namespace Net;

class ConnectionImpl : public SocketConnection {
public:
	ConnectionImpl() : SocketConnection(1024, 1024) {}
	virtual ~ConnectionImpl() {}
	virtual void OnConnected() {
		printf("OnConnected MaxOutBufferSize=%d, MaxInBufferSize=%d\n", GetMaxOutBufferSize(), GetMaxInBufferSize());
		if (Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0")) < 0) {
			printf("Write error\n");
			Shutdown(true);
		}
	}
	virtual void OnConnectFailed(int reason) {
		printf("OnConnectFailed %d %s %s\n", reason, uv_err_name(reason), uv_strerror(reason));
	}
	virtual void OnDisconnect(bool isRemote) {
		printf("OnDisconnect %d\n", isRemote);
	}
	virtual void OnDisconnected(bool isRemote) {
		printf("OnDisconnected %d\n", isRemote);
	}
	virtual void OnNewDataReceived() {
		printf("OnNewDataReceived\n");
		int size = GetRecvDataSize();
		if (size > 0) {
			printf("%s\n", GetRecvData());
			int left_size = size / 2;
			PopRecvData(left_size);
			PopRecvData(size - left_size);
		}
		Shutdown(false);
	}
	virtual void OnSomeDataSent() {
		printf("OnSomeDataSent\n");
	}
};

class AcceptorImpl : public SocketAcceptor {
public:
	AcceptorImpl(EventReactor * reactor) : SocketAcceptor(reactor) {}
	virtual ~AcceptorImpl() {
		while (!list_.empty()) {
			list_.front()->Release();
			list_.pop_front();
		}
	}
	virtual void MakeConnection(SocketConnection * & connection) {
		connection = new ConnectionImpl();
		list_.push_back(connection);
	}
	virtual void OnAccepted(SocketConnection * connection) {
		printf("OnAccepted %s\n", connection->GetSocket()->RemoteAddress().ToString().c_str());
		Close();
	}

private:
	std::list<SocketConnection *> list_;
};

class ConnectionImpl2 : public SocketConnection {
public:
	ConnectionImpl2() : SocketConnection(1024, 1024) {}
	virtual ~ConnectionImpl2() {}
	virtual void OnConnected() {
		printf("OnConnected\n");
		SetMaxOutBufferSize(256);
		SetMaxInBufferSize(2048);
		Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0"));
		Shutdown(false);
	}
	virtual void OnDisconnect(bool isRemote) {
		printf("OnDisconnect %d\n", isRemote);
	}
	virtual void OnDisconnected(bool isRemote) {
		printf("OnDisconnected %d\n", isRemote);
	}
	virtual void OnNewDataReceived() {
		printf("OnNewDataReceived\n");
	}
};

class ConnectionImpl3 : public SocketConnection {
public:
	ConnectionImpl3() : SocketConnection(1024, 1024) {}
	virtual ~ConnectionImpl3() {}
	virtual void OnConnected() {
		printf("OnConnected\n");
		Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0"));
	}
	virtual void OnDisconnect(bool isRemote) {
		printf("OnDisconnect %d\n", isRemote);
	}
	virtual void OnDisconnected(bool isRemote) {
		printf("OnDisconnected %d\n", isRemote);
	}
	virtual void OnNewDataReceived() {
		printf("OnNewDataReceived\n");
		char buf[32];
		int readed = Read(buf, sizeof(buf) - 1);
		buf[readed] = 0;
		printf("%s\n", buf);
	}
};

class ConnectionImpl4 : public SocketConnection {
public:
	ConnectionImpl4() : SocketConnection(128, 128) {}
	virtual ~ConnectionImpl4() {}
	virtual void OnConnected() {
		printf("OnConnected\n");
		Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0"));
		Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0"));
		Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0"));
		Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0"));
	}
	virtual void OnDisconnect(bool isRemote) {
		printf("OnDisconnect %d\n", isRemote);
	}
	virtual void OnDisconnected(bool isRemote) {
		printf("OnDisconnected %d\n", isRemote);
	}
	virtual void OnNewDataReceived() {
		printf("OnNewDataReceived len=%d\n", GetRecvDataSize());
	}
};

class AcceptorImpl4 : public SocketAcceptor {
public:
	AcceptorImpl4(EventReactor * reactor) : SocketAcceptor(reactor) {}
	virtual ~AcceptorImpl4() {
		while (!list_.empty()) {
			list_.front()->Release();
			list_.pop_front();
		}
	}
	virtual void MakeConnection(SocketConnection * & connection) {
		connection = new ConnectionImpl4();
		list_.push_back(connection);
	}
	virtual void OnAccepted(SocketConnection * connection) {
		printf("OnAccepted %s\n", connection->GetSocket()->RemoteAddress().ToString().c_str());
		Close();
	}

private:
	std::list<SocketConnection *> list_;
};

class ConnectionImpl5 : public SocketConnection {
public:
	ConnectionImpl5() : SocketConnection(128, 128), writed_(false) {}
	virtual ~ConnectionImpl5() {}
	virtual void OnConnected() {
		char buf[65536] = {1};
		Write(buf, sizeof(buf));
	}
	virtual void OnDisconnect(bool isRemote) {
	}
	virtual void OnDisconnected(bool isRemote) {
	}
	virtual void OnNewDataReceived() {
		try {
			PopRecvData(2000);
		} catch (std::exception & e) {
			printf("OnNewDataReceived: %s\n", e.what());
		}
		Shutdown(false);
	}
	virtual void OnSomeDataSent() {
		if (!writed_) {
			writed_ = true;
			char buf[2048] = {2};
			Write(buf, sizeof(buf));
		}
	}
private:
	bool writed_;
};

class AcceptorImpl5 : public SocketAcceptor {
public:
	AcceptorImpl5(EventReactor * reactor) : SocketAcceptor(reactor) {}
	virtual ~AcceptorImpl5() {
		while (!list_.empty()) {
			list_.front()->Release();
			list_.pop_front();
		}
	}
	virtual void MakeConnection(SocketConnection * & connection) {
		connection = new ConnectionImpl5();
		list_.push_back(connection);
	}
	virtual void OnAccepted(SocketConnection * connection) {
		printf("OnAccepted %s\n", connection->GetSocket()->RemoteAddress().ToString().c_str());
		Close();
	}

private:
	std::list<SocketConnection *> list_;
};

TEST(ReactorTestSuite, use) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl(pReactor);
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl();
	pAcceptor->Open(SocketAddress(6789));
	pConnector->Connect(pConnection, SocketAddress("127.0.0.1", 6789));
	
	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	pAcceptor->Destroy();
	delete pReactor;
}

TEST(ReactorTestSuite, ConnectError) {
	EventReactor * pReactor = new EventReactor();
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl();
	pConnector->Connect(pConnection, SocketAddress("0.0.0.0", 6789));
	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	delete pReactor;
}

TEST(ReactorTestSuite, ConnectShutdown) {
	EventReactor * pReactor = new EventReactor();
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl();
	pConnector->Connect(pConnection, SocketAddress("0.0.0.0", 6789));
	pConnection->Shutdown(true);
	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	delete pReactor;
}

TEST(ReactorTestSuite, ClearHandler) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl(pReactor);
	pAcceptor->Open(SocketAddress(6789));
	delete pReactor;
	pAcceptor->Destroy();
}

TEST(ReactorTestSuite, use2) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl(pReactor);
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl2();
	pAcceptor->Open(SocketAddress(6789));
	pConnector->Connect(pConnection, SocketAddress("127.0.0.1", 6789));

	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	pAcceptor->Destroy();
	delete pReactor;
}

TEST(ReactorTestSuite, use3) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl(pReactor);
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl3();
	pAcceptor->Open(SocketAddress(6789));
	pConnector->Connect(pConnection, SocketAddress("127.0.0.1", 6789));

	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	pAcceptor->Destroy();
	delete pReactor;
}

TEST(ReactorTestSuite, use4) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl4(pReactor);
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl4();
	pAcceptor->Open(SocketAddress(6789));
	pConnector->Connect(pConnection, SocketAddress("127.0.0.1", 6789));

	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	pAcceptor->Destroy();
	delete pReactor;
}

TEST(ReactorTestSuite, AccpetError) {
	EventReactor reactor;
	SocketAcceptor * acceptor = new AcceptorImpl(&reactor);
	acceptor->Open(SocketAddress(6789));
	acceptor->Open(SocketAddress(6789));
	acceptor->Close();
	reactor.Dispatch(UV_RUN_DEFAULT);
	acceptor->Destroy();
}

TEST(ReactorTestSuite, ConnectError2) {
	EventReactor reactor;
	SocketConnector * connector = new SocketConnector(&reactor);
	SocketConnection * connection = new SocketConnection(1024, 1024);
	connector->Connect(connection, SocketAddress(6789));
	connector->Connect(connection, SocketAddress(6789));
	connection->Destroy();
	reactor.Dispatch(UV_RUN_DEFAULT);
	connector->Destroy();
}

TEST(ReactorTestSuite, ConnectionError) {
	EventReactor reactor;
	SocketConnection * connection = new SocketConnection(1024, 1024);
	connection->SetReactor(&reactor);
	connection->Open();
	connection->SetConnectState(SocketConnection::kConnected);
	connection->Open();
	connection->SetConnectState(SocketConnection::kDisconnected);
	connection->Destroy();
}

TEST(ReactorTestSuite, WriteError) {
	char buf[2048] = {1};
	SocketConnection * connection = new SocketConnection(1024, 1024);
	EXPECT_EQ(connection->Write(nullptr, 0), UV_ENOTCONN);
	connection->SetConnectState(SocketConnection::kConnected);
	connection->GetSocket()->Open(uv_default_loop());
	connection->Write(buf, sizeof(buf));
	connection->SetConnectState(SocketConnection::kDisconnected);
	connection->Destroy();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(ReactorTestSuite, ReadError) {
	char buf[2048] = {1};
	SocketConnection * connection = new SocketConnection(1024, 1024);
	EXPECT_EQ(connection->Read(nullptr, 0), UV_ENOBUFS);
	connection->PopRecvData(1);
	try {
		connection->SetConnectState(SocketConnection::kConnected);
		connection->PopRecvData(1);
	} catch (std::exception & e) {
		printf("ReactorTestSuite - ReadError: %s\n", e.what());
	}
	connection->SetConnectState(SocketConnection::kDisconnected);
	connection->Destroy();
}

TEST(ReactorTestSuite, ReadError2) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl5(pReactor);
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new ConnectionImpl5();
	pAcceptor->Open(SocketAddress(6789));
	pConnector->Connect(pConnection, SocketAddress("127.0.0.1", 6789));

	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Destroy();
	pConnector->Destroy();
	pAcceptor->Destroy();
	delete pReactor;
}