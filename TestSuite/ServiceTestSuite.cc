#include "gtest/gtest.h"
#include "Core/SocketClient.h"
#include "Core/SocketServer.h"
#include "Core/SocketConnectionImpl.h"

using namespace Net;

class TestEvent : public IEvent {
public:
	virtual int OnConnected(SocketWrapper * wrapper) {
		printf("OnConnected %s:%d\n", wrapper->GetMgr()->GetName().c_str(), wrapper->GetId());
		return 0;
	}
	virtual int OnConnectFailed(SocketWrapper * wrapper, int reason) {
		printf("OnConnectFailed %s:%d:%s:%s\n", wrapper->GetMgr()->GetName().c_str(), wrapper->GetId(), uv_err_name(reason), uv_strerror(reason));
		return 0;
	}
	virtual int OnDisconnected(SocketWrapper * wrapper, bool isRemote) {
		printf("OnDisconnected %s:%d:%d\n", wrapper->GetMgr()->GetName().c_str(), wrapper->GetId(), isRemote);
		return 0;
	}
	virtual int OnNewDataReceived(SocketWrapper * wrapper) {
		printf("OnNewDataReceived %s:%d\n", wrapper->GetMgr()->GetName().c_str(), wrapper->GetId());
		return 0;
	}
	virtual int OnSomeDataSent(SocketWrapper * wrapper) {
		printf("OnSomeDataSent %s:%d\n", wrapper->GetMgr()->GetName().c_str(), wrapper->GetId());
		return 0;
	}
};

class MockEvent : public IEvent {
public:
	MockEvent(const char * data) : data_(data) {
		len_ = static_cast<int>(data_.size());
		count_ = 0;
		connected_count_ = 0;
		write_count_ = 0;
	}
	virtual int OnConnected(SocketWrapper * wrapper) {
		printf("OnConnected %d %d %d %d\n", wrapper->GetMaxInBufferSize(), wrapper->GetMaxOutBufferSize(), wrapper->GetOutBufferUsedSize(), ++connected_count_);
		wrapper->Write(data_.c_str(), len_);
		return 0;
	}
	virtual int OnConnectFailed(SocketWrapper * wrapper, int reason) {
		wrapper->ShutdownNow();
		return 0;
	}
	virtual int OnDisconnected(SocketWrapper * wrapper, bool isRemote) {
		return 0;
	}
	virtual int OnNewDataReceived(SocketWrapper * wrapper) {
		printf("OnNewDataReceived %d\n", ++count_);
		if (wrapper->GetRecvDataSize() >= len_) {
			char buf[1024];
			memcpy(buf, wrapper->GetRecvData(), len_);
			wrapper->PopRecvData(len_);
			buf[len_] = 0;
			EXPECT_STREQ(buf, data_.c_str());
			wrapper->Shutdown();
		}
		return 0;
	}
	virtual int OnSomeDataSent(SocketWrapper * wrapper) {
		printf("OnConnected %d %p\n", ++write_count_, wrapper);
		if (write_count_ < 100) {
			wrapper->Write(data_.c_str(), len_);
		}
		return 0;
	}

private:
	std::string data_;
	int len_;
	int count_;
	int write_count_;
	int connected_count_;
};

class MockEvent2 : public IEvent {
public:
	virtual int OnConnected(SocketWrapper * wrapper) {
		wrapper->Write("this is a test message", sizeof("this is a test message"));
		return 0;
	}
	virtual int OnConnectFailed(SocketWrapper * wrapper, int reason) {
		return 0;
	}
	virtual int OnDisconnected(SocketWrapper * wrapper, bool isRemote) {
		wrapper->SetMaxInBufferSize(256);
		return 0;
	}
	virtual int OnNewDataReceived(SocketWrapper * wrapper) {
		char buf[1024];
		int readed = wrapper->Read(buf, sizeof(buf) - 1);
		buf[readed] = 0;
		printf("OnNewDataReceived: %s\n", buf);
		if (wrapper->GetMgr()->GetName() == "WR2Server") {
			dynamic_cast<SocketServer *>(wrapper->GetMgr())->Terminate();
		} else {
			wrapper->ShutdownNow();
		}
		return 0;
	}
	virtual int OnSomeDataSent(SocketWrapper * wrapper) {
		wrapper->SetMaxOutBufferSize(256);
		return 0;
	}
};

class MockEvent3 : public IEvent {
public:
	MockEvent3() {
		count_ = 0;
	}
	virtual int OnConnected(SocketWrapper * wrapper) {
		char buf[2048] = {1};
		wrapper->Write(buf, sizeof(buf));
		return 0;
	}
	virtual int OnConnectFailed(SocketWrapper * wrapper, int reason) {
		return 0;
	}
	virtual int OnDisconnected(SocketWrapper * wrapper, bool isRemote) {
		if (wrapper->GetMgr()->GetName() == "WR3Server") {
			dynamic_cast<SocketServer *>(wrapper->GetMgr())->Terminate();
		} else {
			wrapper->ShutdownNow();
		}
		return 0;
	}
	virtual int OnNewDataReceived(SocketWrapper * wrapper) {
		if (++count_ > 10) {
			wrapper->Shutdown();
		}
		return 0;
	}
	virtual int OnSomeDataSent(SocketWrapper * wrapper) {
		if (++count_ < 2) {
			char buf[2048] = {1};
			wrapper->Write(buf, sizeof(buf));
		}
		return 0;
	}
private:
	int count_;
};

class ErrorEvent : public IEvent {
public:
	virtual int OnConnected(SocketWrapper * wrapper) {
		wrapper->Write("hello", sizeof("hello"));
		return 1;
	}
	virtual int OnConnectFailed(SocketWrapper * wrapper, int reason) {
		return 1;
	}
	virtual int OnDisconnected(SocketWrapper * wrapper, bool isRemote) {
		return 1;
	}
	virtual int OnNewDataReceived(SocketWrapper * wrapper) {
		return 1;
	}
	virtual int OnSomeDataSent(SocketWrapper * wrapper) {
		return 1;
	}
};

class ErrorEvent2 : public ErrorEvent {
public:
	virtual int OnConnected(SocketWrapper * wrapper) {
		wrapper->Write("hello", sizeof("hello"));
		return 0;
	}
};

class ErrorEvent3 : public ErrorEvent2 {
public:
	virtual int OnSomeDataSent(SocketWrapper * wrapper) {
		return 0;
	}
};

class MockClient : public SocketClient {
public:
	MockClient(EventReactor * reactor) : SocketClient("MockClient", reactor, nullptr, 1024, 1024) {
	}
	virtual ~MockClient() {
	}
	virtual bool Connect(const std::string & address, int port, u32 & id) {
		id = 0;
		if (!connector_) {
			connector_ = new SocketConnector(reactor_);
		}
		SocketWrapper * wrapper = new SocketWrapper(this, max_out_buffer_size_, max_in_buffer_size_);
		wrapper->SetId(1024);
		id = Register(wrapper);
		SocketAddress sa(address, static_cast<u16>(port));
		int status = connector_->Connect(wrapper->GetConnection(), sa);
		if (status < 0) {
			printf("%s: 建立连接[%s]失败\n", GetName().c_str(), sa.ToString().c_str());
			return false;
		}
		return true;
	}
};

struct Context {
	EventReactor * reactor;
	SocketConnector * connector;
	std::vector<SocketClient *> * client_list_;
};

void ServiceTestSuite_close_cb(uv_handle_t* handle) {
	delete handle;
}

void ServiceTestSuite_timer_cb(uv_timer_t* handle) {
	uv_stop(handle->loop);
	uv_close(reinterpret_cast<uv_handle_t*>(handle), ServiceTestSuite_close_cb);
}

void ServiceTestSuite_timer_cb2(uv_timer_t* handle) {
	Context * context = static_cast<Context *>(handle->data);
	if (context->client_list_->size() >= 100) {
		uv_stop(handle->loop);
		uv_close(reinterpret_cast<uv_handle_t*>(handle), ServiceTestSuite_close_cb);
		return;
	}
	u32 id;
	SocketClient * client = new SocketClient("SocketClient", context->reactor, context->connector, 256, 256);
	client->Connect("127.0.0.1", 6789, id);
	context->client_list_->push_back(client);
}

void ServiceTestSuite_timer_cb3(uv_timer_t* handle) {
	SocketServer * server = static_cast<SocketServer *>(handle->data);
	if (server) {
		server->Terminate();
	}
	uv_close(reinterpret_cast<uv_handle_t*>(handle), ServiceTestSuite_close_cb);
}

TEST(ServiceTestSuite, client) {
	TestEvent event;
	EventReactor reactor;
	SocketConnector * connector = new SocketConnector(&reactor);
	SocketClient client("TestClient", &reactor, connector, 1024, 1024);
	client.SetEvent(&event);
	SocketClient client2("TestClient2", &reactor, connector, 1024, 1024);
	client2.SetEvent(&event);
	MockClient client3(&reactor);
	client3.SetEvent(&event);
	u32 id = 0;
	EXPECT_EQ(client.Connect("127.0.0.1", 6789, id), true);
	EXPECT_EQ(id, 1);
	client2.Connect("127.0.0.1", 6789, id);
	client2.ShutDownOneSocketWrapper(id);
	EXPECT_EQ(client3.Connect("127.0.0.1", 6789, id), true);
	EXPECT_EQ(id, 1024);
	reactor.Dispatch(UV_RUN_DEFAULT);
	connector->Destroy();
}

TEST(ServiceTestSuite, server) {
	TestEvent event;
	EventReactor reactor;
	SocketServer server("TestServer", &reactor, 1024, 1024);
	server.SetEvent(&event);
	EXPECT_EQ(server.Listen("0.0.0.0", 6789), true);
	EXPECT_EQ(server.Listen("::", 6789), false);
	EXPECT_EQ(server.Terminate(), true);
	EXPECT_EQ(server.Terminate(), false);
	EXPECT_EQ(server.Listen("0.0.0.0", 6789, 128, true), false);
}

TEST(ServiceTestSuite, connect) {
	TestEvent event;
	EventReactor reactor;
	SocketServer server("TestServer", &reactor, 1024, 1024);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);
	SocketClient client("TestClient", &reactor, nullptr, 1024, 1024);
	client.SetEvent(&event);
	u32 id = 0;
	client.Connect("127.0.0.1", 6789, id);

	uv_timer_t * t = new uv_timer_t();
	uv_timer_init(reactor.GetEventLoop(), t);
	uv_timer_start(t, ServiceTestSuite_timer_cb, 5000, 0);
	reactor.Dispatch(UV_RUN_DEFAULT);
}

TEST(ServiceTestSuite, use) {
	TestEvent event;
	EventReactor reactor;
	SocketServer server("TestServer", &reactor, 1024, 1024);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);

	Context context;
	context.reactor = &reactor;
	context.connector = new SocketConnector(&reactor);
	context.client_list_ = new std::vector<SocketClient *>();
	context.client_list_->reserve(1024);
	uv_timer_t * t = new uv_timer_t();
	uv_timer_init(reactor.GetEventLoop(), t);
	t->data = static_cast<void *>(&context);
	uv_timer_start(t, ServiceTestSuite_timer_cb2, 10, 10);
	reactor.Dispatch(UV_RUN_DEFAULT);
	EXPECT_EQ(server.GetSocketWrapperCount(), static_cast<u32>(context.client_list_->size()));
	for (auto & it : *context.client_list_) {
		delete it;
	}
	delete context.client_list_;
	context.connector->Destroy();
}

TEST(ServiceTestSuite, WR) {
	MockEvent event("hello world");
	EventReactor reactor;
	SocketServer server("WRServer", &reactor, 1024, 1024);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);

	std::list<SocketClient *> client_list;
	SocketConnector * connector = new SocketConnector(&reactor);
	u32 id;
	for (int i = 0; i < 10; ++i) {
		SocketClient * client = new SocketClient("WRClient", &reactor, connector, 1024, 1024);
		client->SetEvent(&event);
		EXPECT_EQ(true, client->Connect("127.0.0.1", 6789, id));
		EXPECT_EQ(id, 1);
		client_list.push_back(client);
	}
	uv_timer_t * t = new uv_timer_t();
	uv_timer_init(reactor.GetEventLoop(), t);
	t->data = static_cast<void *>(&server);
	uv_timer_start(t, ServiceTestSuite_timer_cb3, 5000, 0);
	reactor.Dispatch(UV_RUN_DEFAULT);
	for (auto & it : client_list) {
		delete it;
	}
	connector->Destroy();
}

TEST(ServiceTestSuite, WR2) {
	MockEvent2 event;
	EventReactor reactor;
	SocketServer server("WR2Server", &reactor, 128, 128);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);

	SocketClient client("WR2Client", &reactor, nullptr, 128, 128);
	client.SetEvent(&event);
	u32 id;
	client.Connect("127.0.0.1", 6789, id);
	reactor.Dispatch(UV_RUN_DEFAULT);
}

TEST(ServiceTestSuite, Error) {
	try {
		SocketWrapper wrapper(nullptr, 1024, 1024);
	} catch (std::exception & e) {
		printf("ServiceTestSuite - Error: %s\n", e.what());
	}
	
	try {
		SocketConnectionImpl * impl = new SocketConnectionImpl(nullptr, 1024, 1024);
	} catch (std::exception & e) {
		printf("ServiceTestSuite - Error: %s\n", e.what());
	}
}

TEST(ServiceTestSuite, WR3) {
	MockEvent3 event;
	EventReactor reactor;
	SocketServer server("WR3Server", &reactor, 128, 128);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);

	SocketClient client("WR3Client", &reactor, nullptr, 128, 128);
	client.SetEvent(&event);
	u32 id;
	client.Connect("127.0.0.1", 6789, id);
	reactor.Dispatch(UV_RUN_DEFAULT);
}

TEST(ServiceTestSuite, CB) {
	ErrorEvent event;
	EventReactor reactor;
	SocketServer server("CBServer", &reactor, 128, 128);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);
	SocketClient client("CBClient", &reactor, nullptr, 128, 128);
	client.SetEvent(&event);
	u32 id;
	client.Connect("127.0.0.1", 6789, id);
	int loop_num = 10;
	while (--loop_num > 0) {
		reactor.Dispatch();
	}
}

TEST(ServiceTestSuite, CB2) {
	ErrorEvent2 event;
	EventReactor reactor;
	SocketServer server("CB2Server", &reactor, 128, 128);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);
	SocketClient client("CB2Client", &reactor, nullptr, 128, 128);
	client.SetEvent(&event);
	u32 id;
	client.Connect("127.0.0.1", 6789, id);
	int loop_num = 10;
	while (--loop_num > 0) {
		reactor.Dispatch();
	}
}

TEST(ServiceTestSuite, CB3) {
	ErrorEvent3 event;
	EventReactor reactor;
	SocketServer server("CB3Server", &reactor, 128, 128);
	server.SetEvent(&event);
	server.Listen("0.0.0.0", 6789);
	SocketClient client("CB3Client", &reactor, nullptr, 128, 128);
	client.SetEvent(&event);
	u32 id;
	client.Connect("127.0.0.1", 6789, id);
	int loop_num = 10;
	while (--loop_num > 0) {
		reactor.Dispatch();
	}
}