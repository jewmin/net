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