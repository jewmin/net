#include "gtest/gtest.h"
#include "Sockets/Socket.h"
#include "Sockets/ServerSocket.h"
#include "Sockets/StreamSocket.h"
#include "Sockets/ServerSocketImpl.h"
#include "Sockets/StreamSocketImpl.h"
#include "Common/Allocator.h"

void PrintRefCount(Net::UvData * data, const char * extra_msg) {
	std::printf("%p refcount = %d, %s\n", data, data->ReferenceCount(), extra_msg);
}

class SocketTestSuiteMockCopySocket : public Net::Socket {
public:
	SocketTestSuiteMockCopySocket(Net::SocketImpl * impl) : Net::Socket(impl) {}
	virtual ~SocketTestSuiteMockCopySocket() {}
};

class SocketTestSuiteMockSocketImpl : public Net::SocketImpl {
public:
	SocketTestSuiteMockSocketImpl() : Net::SocketImpl() {}
	virtual ~SocketTestSuiteMockSocketImpl() {}
	uv_tcp_t * GetTcp() const { return reinterpret_cast<uv_tcp_t *>(handle_); }
};

class SocketTestSuiteMockSocket : public Net::Socket {
public:
	SocketTestSuiteMockSocket() : Net::Socket(new SocketTestSuiteMockSocketImpl()) {}
	virtual ~SocketTestSuiteMockSocket() {}
};

class SocketTestSuiteMockStreamSocket : public Net::StreamSocket {
public:
	SocketTestSuiteMockStreamSocket(Net::SocketImpl * impl) : Net::StreamSocket(impl) {}
	virtual ~SocketTestSuiteMockStreamSocket() {}
};

class SocketTestSuiteMockServerSocketImpl : public Net::ServerSocketImpl {
public:
	SocketTestSuiteMockServerSocketImpl() {}
	virtual ~SocketTestSuiteMockServerSocketImpl() {}
	uv_tcp_t * GetTcp() const { return reinterpret_cast<uv_tcp_t *>(handle_); }
};

TEST(SocketTestSuite, Construct) {
	Net::Socket s1;
	Net::Socket s2(s1);
	Net::Socket s3;
	SocketTestSuiteMockSocket s4;
	s3 = s1;

	SocketTestSuiteMockCopySocket ss1(new Net::ServerSocketImpl());
	Net::ServerSocket ss2(ss1);
	Net::ServerSocket ss3, ss4(ss3);
	ss3 = ss1;
	ss4 = ss3;

	SocketTestSuiteMockCopySocket cs1(new Net::StreamSocketImpl());
	Net::StreamSocket cs2(cs1);
	Net::StreamSocket cs3, cs4(cs3);
	cs3 = cs1;
	cs4 = cs3;
	SocketTestSuiteMockStreamSocket cs5(new Net::StreamSocketImpl());
}

TEST(SocketTestSuite, ConstructCatch) {
	SocketTestSuiteMockSocket ms;
	try {
		Net::ServerSocket s1(ms);
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		Net::ServerSocket s2;
		s2 = ms;
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		Net::StreamSocket s3(ms);
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		Net::StreamSocket s4;
		s4 = ms;
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		SocketTestSuiteMockStreamSocket s5(new Net::ServerSocketImpl());
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		SocketTestSuiteMockCopySocket s6(nullptr);
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	// try {
	// 	Net::StreamSocket s7;
	// 	s7.SetUvData(nullptr);
	// } catch (std::exception & e) {
	// 	printf("SocketTestSuite - Catch: %s\n", e.what());
	// }
}

TEST(SocketTestSuite, Equal) {
	int size = 4;
	Net::StreamSocketImpl * o = (Net::StreamSocketImpl *)jc_malloc(sizeof(*o) * size);
	for (int i = 0; i < size; ++i) {
		new(o + i)Net::StreamSocketImpl();
		(o + i)->Duplicate();
		(o + i)->Duplicate();
	}

	{
		SocketTestSuiteMockStreamSocket s1(o), ss1(o);
		SocketTestSuiteMockStreamSocket s2(o + 1), ss2(o + 1);
		SocketTestSuiteMockStreamSocket s3(o + 2), ss3(o + 2);
		SocketTestSuiteMockStreamSocket s4(o + 3), ss4(o + 3);
		EXPECT_EQ(s1, ss1);
		EXPECT_EQ(s2, ss2);
		EXPECT_EQ(s3, ss3);
		EXPECT_EQ(s4, ss4);

		EXPECT_NE(s1, ss2);
		EXPECT_NE(s1, ss3);
		EXPECT_NE(s1, ss4);

		EXPECT_GT(s2, ss1);
		EXPECT_GT(s3, ss1);
		EXPECT_GT(s4, ss1);

		EXPECT_GE(s1, ss1);
		EXPECT_GE(s2, ss1);
		EXPECT_GE(s3, ss1);
		EXPECT_GE(s4, ss1);

		EXPECT_LT(s1, ss4);
		EXPECT_LT(s2, ss4);
		EXPECT_LT(s3, ss4);

		EXPECT_LE(s1, ss4);
		EXPECT_LE(s2, ss4);
		EXPECT_LE(s3, ss4);
		EXPECT_LE(s4, ss4);
	}

	for (int i = 0; i < size; ++i) {
		(o + i)->~StreamSocketImpl();
	}
	jc_free(o);
}

class SocketTestSuiteMockMockUvData : public Net::UvData {
public:
	SocketTestSuiteMockMockUvData() {
		rb = uv_buf_init((char *)jc_malloc(1024), 1024);
		wb = uv_buf_init((char *)jc_malloc(128), 128);
	}
	virtual ~SocketTestSuiteMockMockUvData() {
		server.Close();
		client.Close();
		jc_free(rb.base);
		jc_free(wb.base);
	}
	void CloseCallback() override {
		PrintRefCount(this, "closed");
	}
	void AcceptCallback(i32 status) override {
		if (status == 0 && server.AcceptSocket(client)) {
			client.SetNoDelay();
			std::printf("accept success: %s\n", client.RemoteAddress().ToString().c_str());
			client.SetSendBufferSize(256);
			client.SetRecvBufferSize(256);
			client.Established();
			client.SetUvData(this);
		} else {
			client.Close();
			server.Close();
		}
	}
	void ConnectCallback(i32 status, void * arg) override {
		if (status == 0) {
			client.SetKeepAlive(60);
			std::printf("connect success: %s %d %d\n", client.LocalAddress().ToString().c_str(), client.GetSendBufferSize(), client.GetRecvBufferSize());
			wb.len = sizeof("this is a test message\0");
			std::memcpy(wb.base, "this is a test message\0", wb.len);
			client.Write(wb.base, wb.len);
			std::printf("write queue: %d\n", client.GetWriteQueueSize());
			Duplicate();
		} else {
			client.Close();
			server.Close();
		}
	}
	void ShutdownCallback(i32 status, void * arg) override {
		client.Close();
	}
	void AllocCallback(uv_buf_t * buf) override {
		buf->base = rb.base;
		buf->len = rb.len;
	}
	void ReadCallback(i32 status) override {
		if (status > 0) {
			if (status >= static_cast<i32>(rb.len)) { status = rb.len - 1; }
			rb.base[status] = 0;
			std::printf("%s\n", rb.base);
		}
		client.ShutdownRead();
		client.ShutdownWrite();
		server.Close();
	}
	void WrittenCallback(i32 status, void * arg) override {
		client.Shutdown();
		server.Close();
	}
	Net::ServerSocket server;
	Net::StreamSocket client;
	uv_buf_t rb;
	uv_buf_t wb;
};

class SocketTestSuiteCallback : public Net::UvData {
public:
	virtual ~SocketTestSuiteCallback() {
		server.Close();
		client.Close();
	}
	virtual void AcceptCallback(i32 status) {
		server.AcceptSocket(client);
		client.Established();
	}
	virtual void ConnectCallback(i32 status, void * arg) {
		Duplicate();
		client.SetUvData(nullptr);
		static char buf[32];
		std::memset(buf, 0, sizeof(buf));
		std::memcpy(buf, "this is a test message", sizeof("this is a test message"));
		client.Write(buf, sizeof(buf));
		client.Shutdown();
	}
	virtual void ShutdownCallback(i32 status, void * arg) {}
	virtual void AllocCallback(uv_buf_t * buf) {}
	virtual void ReadCallback(i32 status) {}
	virtual void WrittenCallback(i32 status, void * arg) {}

	Net::ServerSocket server;
	Net::StreamSocket client;
};

class SocketTestSuiteTest : public testing::Test {
protected:
	virtual void SetUp() {
		uv_loop_init(&loop_);
	}

	virtual void TearDown() {
		uv_run(&loop_, UV_RUN_DEFAULT);
		uv_loop_close(&loop_);
	}

	uv_loop_t * GetLoop() { return &loop_; }

private:
	uv_loop_t loop_;
};

TEST_F(SocketTestSuiteTest, Operation) {
	SocketTestSuiteMockMockUvData * server_data = new SocketTestSuiteMockMockUvData();
	SocketTestSuiteMockMockUvData * client_data = new SocketTestSuiteMockMockUvData();
	PrintRefCount(server_data, "new server");
	PrintRefCount(client_data, "new client");
	server_data->server.Open(GetLoop());
	server_data->server.Bind(Net::SocketAddress(Net::IPAddress("0.0.0.0"), 6789));
	server_data->server.Listen();
	server_data->server.SetUvData(server_data);

	client_data->client.Open(GetLoop());
	client_data->client.Bind(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 9999));
	client_data->client.Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789));
	client_data->client.SetUvData(client_data);

	PrintRefCount(server_data, "before run");
	PrintRefCount(client_data, "before run");
	uv_run(GetLoop(), UV_RUN_DEFAULT);
	PrintRefCount(server_data, "after run");
	PrintRefCount(client_data, "after run");

	client_data->Destroy();
	server_data->Destroy();
}

TEST_F(SocketTestSuiteTest, Callback) {
	Net::ServerSocket server;
	Net::StreamSocket client;
	server.Open(GetLoop());
	server.Bind(Net::SocketAddress(Net::IPAddress("0.0.0.0"), 6789));
	server.Listen();
	client.Open(GetLoop());
	client.Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789));

	SocketTestSuiteCallback * server_data = new SocketTestSuiteCallback();
	SocketTestSuiteCallback * client_data = new SocketTestSuiteCallback();
	server_data->server.Open(GetLoop());
	server_data->server.Bind(Net::SocketAddress(Net::IPAddress("0.0.0.0"), 7890));
	server_data->server.Listen();
	server_data->server.SetUvData(server_data);

	client_data->client.Open(GetLoop());
	client_data->client.Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 7890));
	client_data->client.SetUvData(client_data);

	int count = 3;
	while (count-- > 0) {
		uv_run(GetLoop(), UV_RUN_NOWAIT);
	}

	server.Close();
	client.Close();
	server_data->server.Close();
	server_data->Destroy();
	client_data->Destroy();
}

class SocketTestSuiteImplTest : public SocketTestSuiteTest {
protected:
	virtual void SetUp() {
		SocketTestSuiteTest::SetUp();
		impl.Open(GetLoop());
	}

	virtual void TearDown() {
		impl.Close();
		SocketTestSuiteTest::TearDown();
	}

	SocketTestSuiteMockSocketImpl * GetImpl() { return &impl; }

private:
	SocketTestSuiteMockSocketImpl impl;
};

TEST_F(SocketTestSuiteImplTest, Bind) {
	EXPECT_LT(GetImpl()->Bind(Net::SocketAddress(Net::IPAddress("0.0.0.0"), 6789), true, true), 0);
}

TEST_F(SocketTestSuiteImplTest, Listen) {
	uv_tcp_t * tcp = GetImpl()->GetTcp();
	tcp->delayed_error = UV_ECANCELED;
	EXPECT_EQ(GetImpl()->Listen(), UV_ECANCELED);
}

#ifdef _WIN32
TEST_F(SocketTestSuiteImplTest, Connect) {
	uv_tcp_t * tcp = GetImpl()->GetTcp();
	tcp->delayed_error = UV_ECANCELED;
	EXPECT_LT(GetImpl()->Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789)), 0);
}
#else
TEST_F(SocketTestSuiteImplTest, Connect) {
	EXPECT_EQ(GetImpl()->Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789)), 0);
	EXPECT_LT(GetImpl()->Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789)), 0);
}
#endif

TEST_F(SocketTestSuiteImplTest, Shutdown) {
	EXPECT_EQ(GetImpl()->ShutdownRead(), 0);
	EXPECT_LT(GetImpl()->ShutdownWrite(), 0);
}

TEST_F(SocketTestSuiteImplTest, Established) {
	EXPECT_LT(GetImpl()->Established(), 0);
}

TEST_F(SocketTestSuiteImplTest, BufferSize) {
	GetImpl()->SetSendBufferSize(-1);
	GetImpl()->SetRecvBufferSize(-1);
	uv_tcp_t * tcp = GetImpl()->GetTcp();
	tcp->type = UV_TTY;
	EXPECT_EQ(GetImpl()->GetSendBufferSize(), 0);
	EXPECT_EQ(GetImpl()->GetRecvBufferSize(), 0);
	tcp->type = UV_TCP;
}

TEST_F(SocketTestSuiteImplTest, Address) {
	EXPECT_EQ(GetImpl()->LocalAddress(), Net::SocketAddress());
	EXPECT_EQ(GetImpl()->RemoteAddress(), Net::SocketAddress());
}

TEST_F(SocketTestSuiteImplTest, Write) {
	Net::StreamSocket s;
	EXPECT_EQ(s.Write("123", sizeof("123")), UV_EPROTONOSUPPORT);
	EXPECT_EQ(GetImpl()->Write(nullptr, 100), UV_ENOBUFS);
	EXPECT_EQ(GetImpl()->Write("123", 0), UV_ENOBUFS);
	EXPECT_LT(GetImpl()->Write("123", sizeof("123")), 0);
}

#ifdef _WIN32
TEST_F(SocketTestSuiteImplTest, Misc) {
	uv_tcp_t * tcp = GetImpl()->GetTcp();
	tcp->socket = 0;
	GetImpl()->SetNoDelay();
	GetImpl()->SetKeepAlive(60);
}
#else
TEST_F(SocketTestSuiteImplTest, Misc) {
	uv_tcp_t * tcp = GetImpl()->GetTcp();
	tcp->io_watcher.fd = 0;
	GetImpl()->SetNoDelay();
	GetImpl()->SetKeepAlive(60);
}
#endif

class SocketTestSuiteError : public Net::UvData {
public:
	SocketTestSuiteError() {
		impl_ = new SocketTestSuiteMockServerSocketImpl();
		SocketTestSuiteMockCopySocket socket(impl_);
		socket_ = socket;
	}
	virtual ~SocketTestSuiteError() {
		jc_free(tcp_);
	}
	virtual void CloseCallback() {
		tcp_->type = UV_UDP;
	}
	virtual void AcceptCallback(i32 status) {
		tcp_ = impl_->GetTcp();
#ifdef _WIN32
		uv_tcp_accept_t* req = tcp_->tcp.serv.pending_accepts;
		req->accept_socket = -1;
#else
		tcp_->accepted_fd = -1;
#endif
		Net::StreamSocket client;
		socket_.AcceptSocket(client);
		socket_.Close();
	}
	uv_tcp_t * tcp_;
	SocketTestSuiteMockServerSocketImpl * impl_;
	Net::ServerSocket socket_;
};

TEST_F(SocketTestSuiteTest, CloseCatch) {
	Net::StreamSocket client;
	SocketTestSuiteError * data = new SocketTestSuiteError();
	data->socket_.Open(GetLoop());
	data->socket_.Bind(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789));
	data->socket_.Listen();
	data->socket_.SetUvData(data);
	client.Open(GetLoop());
	client.Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789));
	try {
		uv_run(GetLoop(), UV_RUN_DEFAULT);
	} catch (std::exception & e) {
		std::printf("SocketTestSuiteTest - CloseCatch: %s\n", e.what());
	}
	client.Close();
	data->Destroy();
}