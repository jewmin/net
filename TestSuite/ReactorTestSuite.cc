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
		if (Write("this is a test connection impl message\0", sizeof("this is a test connection impl message\0")) < 0) {
			printf("Write error\n");
			Shutdown(true);
		}
	}
	virtual void OnConnectFailed(int reason) {
		printf("OnConnectFailed %d\n", reason);
	}
	virtual void OnDisconnect(bool isRemote) {
		printf("OnConnectFailed %d\n", isRemote);
	}
	virtual void OnDisconnected(bool isRemote) {
		printf("OnConnectFailed %d\n", isRemote);
	}
	virtual void OnNewDataReceived() {
		printf("OnNewDataReceived\n");
		int size = GetRecvDataSize();
		if (size > 0) {
			printf("%s\n", GetRecvData());
			PopRecvData(size);
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
		connection = new SocketConnection(1024, 1024);
		list_.push_back(connection);
	}
	virtual void OnAccepted(SocketConnection * connection) {
		printf("OnAccepted %s\n", connection->GetSocket()->RemoteAddress().ToString().c_str());
	}

private:
	std::list<SocketConnection *> list_;
};

TEST(ReactorTestSuite, use) {
	EventReactor * pReactor = new EventReactor();
	SocketAcceptor * pAcceptor = new AcceptorImpl(pReactor);
	SocketConnector * pConnector = new SocketConnector(pReactor);
	SocketConnection * pConnection = new SocketConnection(1024, 1024);
	pAcceptor->Open(SocketAddress(6789));
	pConnector->Connect(pConnection, SocketAddress("127.0.0.1", 6789));
	
	pReactor->Dispatch(UV_RUN_DEFAULT);
	pConnection->Release();
	pConnector->Release();
	pAcceptor->Release();
	delete pReactor;
}