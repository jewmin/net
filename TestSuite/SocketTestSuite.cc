#include "gtest/gtest.h"
#include "Sockets/Socket.h"
#include "Sockets/ServerSocket.h"
#include "Sockets/StreamSocket.h"
#include "Sockets/ServerSocketImpl.h"
#include "Sockets/StreamSocketImpl.h"
#include "Common/Allocator.h"

class MockSocketImpl : public Net::SocketImpl {
public:
	MockSocketImpl() : Net::SocketImpl() {}
	virtual ~MockSocketImpl() {}
};

class MockSocket : public Net::Socket {
public:
	MockSocket() : Net::Socket(new MockSocketImpl()) {}
	virtual ~MockSocket() {}
};

class MockStreamSocket : public Net::StreamSocket {
public:
	MockStreamSocket(Net::SocketImpl * impl) : Net::StreamSocket(impl) {}
	virtual ~MockStreamSocket() {}
};

class MockUvData : public Net::UvData {
public:
	MockUvData() {
		rb = uv_buf_init((char *)jc_malloc(1024), 1024);
		wb = uv_buf_init((char *)jc_malloc(128), 128);
	}
	virtual ~MockUvData() {
		jc_free(rb.base);
		jc_free(wb.base);
	}
	void AcceptCallback(i32 status) override {
		if (status == 0 && ss.AcceptSocket(s)) {
			s.SetNoDelay();
			std::printf("accept success: %s\n", s.RemoteAddress().ToString().c_str());
			s.SetSendBufferSize(256);
			s.SetRecvBufferSize(256);
			s.Established();
			s.SetUvData(this);
		} else {
			s.Close();
			ss.Close();
		}
	}
	void ConnectCallback(i32 status, void * arg) override {
		if (status == 0) {
			s.SetKeepAlive(60);
			std::printf("connect success: %s %d %d\n", s.LocalAddress().ToString().c_str(), s.GetSendBufferSize(), s.GetRecvBufferSize());
			wb.len = sizeof("this is a test message\0");
			std::memcpy(wb.base, "this is a test message\0", wb.len);
			s.Write(wb.base, wb.len);
			std::printf("write queue: %d\n", s.GetWriteQueueSize());
			s.SetUvData(this);
		} else {
			s.Close();
			ss.Close();
		}
	}
	void ShutdownCallback(i32 status, void * arg) override {
		s.Close();
	}
	void AllocCallback(uv_buf_t * buf) override {
		buf->base = rb.base;
		buf->len = rb.len;
	}
	void ReadCallback(i32 status) override {
		if (status > 0) {
			if (status >= rb.len) { status = rb.len - 1; }
			rb.base[status] = 0;
			std::printf("%s\n", rb.base);
		}
		s.ShutdownRead();
		s.ShutdownWrite();
		ss.Close();
	}
	void WrittenCallback(i32 status, void * arg) override {
		s.Shutdown();
	}
	Net::ServerSocket ss;
	Net::StreamSocket s;
	uv_buf_t rb;
	uv_buf_t wb;
};

TEST(SocketTestSuite, Construct) {
	Net::Socket s1;
	Net::Socket s2(s1);
	Net::Socket s3;
	MockSocket s4;
	s3 = s1;

	Net::ServerSocket ss1;
	Net::ServerSocket ss2(ss1);
	Net::ServerSocket ss3;
	ss3 = ss1;

	Net::StreamSocket cs1;
	Net::StreamSocket cs2(cs1);
	Net::StreamSocket cs3;
	cs3 = cs1;
	MockStreamSocket cs4(new Net::StreamSocketImpl());
}

TEST(SocketTestSuite, ConstructCatch) {
	MockSocket ms;
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
		MockStreamSocket s5(new Net::ServerSocketImpl());
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}
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
		MockStreamSocket s1(o), ss1(o);
		MockStreamSocket s2(o + 1), ss2(o + 1);
		MockStreamSocket s3(o + 2), ss3(o + 2);
		MockStreamSocket s4(o + 3), ss4(o + 3);
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

		EXPECT_LT(s2, ss4);
		EXPECT_LT(s3, ss4);
		EXPECT_LT(s4, ss4);

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

TEST(SocketTestSuite, Operation) {
	uv_loop_t loop;
	uv_loop_init(&loop);
	MockUvData * server_data = new MockUvData();
	MockUvData * client_data = new MockUvData();
	server_data->ss.Open(&loop);
	server_data->ss.Bind(Net::SocketAddress(Net::IPAddress("0.0.0.0"), 6789));
	server_data->ss.Listen();
	server_data->ss.SetUvData(server_data);

	client_data->s.Open(&loop);
	client_data->s.Connect(Net::SocketAddress(Net::IPAddress("127.0.0.1"), 6789));
	client_data->s.SetUvData(client_data);

	uv_run(&loop, UV_RUN_DEFAULT);
	client_data->Destroy();
	server_data->Destroy();
	uv_loop_close(&loop);
}

TEST(SocketTestSuite, Impl) {
// 	uv_loop_t loop;
// 	uv_loop_init(&loop);
// 	MockSocketImpl * impl = new MockSocketImpl();

// 	try {
// 		impl->Open(&loop);
// 		impl->Attach(nullptr);
// 	} catch (std::exception & e) {
// 		printf("SocketTestSuite - ImplCatch: %s\n", e.what());
// 	}

// 	impl->Established(nullptr, nullptr);
// 	impl->LocalAddress();
// 	impl->RemoteAddress();
// 	impl->GetSendBufferSize();
// 	impl->GetReceiveBufferSize();
// 	impl->SetSendBufferSize(10);
// 	impl->SetReceiveBufferSize(10);
// 	impl->Send(nullptr, 0, nullptr, nullptr);
// 	uv_write_t * w_req = new uv_write_t();
// 	impl->Send("hello", sizeof("hello"), w_req, nullptr);
// 	delete w_req;
// 	uv_shutdown_t * req = new uv_shutdown_t();
// 	impl->ShutdownSend(req, nullptr);
// 	delete req;
// 	impl->Close();
// 	impl->Send(nullptr, 0, nullptr, nullptr);

// 	impl->Release();
// 	uv_run(&loop, UV_RUN_DEFAULT);
// 	uv_loop_close(&loop);
// }

// TEST(SocketTestSuite, AcceptError) {
// 	uv_loop_t loop;
// 	uv_loop_init(&loop);
// 	ServerSocket socket;
// 	socket.Open(&loop);
// 	StreamSocket client;
// 	EXPECT_EQ(socket.AcceptConnection(client), false);
// 	socket.Close();
// 	uv_run(&loop, UV_RUN_DEFAULT);
// 	uv_loop_close(&loop);
}

// void SocketTestSuite_connect_cb(uv_connect_t* req, int status) {
// 	delete req;
// }

// void SocketTestSuite_listen_cb(uv_stream_t* server, int status) {
	
// }

// TEST(SocketTestSuite, ConnectError) {
// 	uv_loop_t loop;
// 	uv_loop_init(&loop);
// 	StreamSocket client;
// 	client.Open(&loop);
// 	uv_connect_t * req1 = new uv_connect_t();
// 	uv_connect_t * req2 = new uv_connect_t();
// 	client.Connect(SocketAddress(IPAddress("127.0.0.1"), 6789), req1, SocketTestSuite_connect_cb);
// 	client.Connect(SocketAddress(IPAddress("127.0.0.1"), 6789), req2, SocketTestSuite_connect_cb);
// 	delete req2;
// 	client.Close();
// 	uv_run(&loop, UV_RUN_DEFAULT);
// 	uv_loop_close(&loop);
// }

// TEST(SocketTestSuite, handleError) {
// 	uv_loop_t loop;
// 	uv_loop_init(&loop);
// 	MockSocketImpl * impl = new MockSocketImpl();
// 	impl->Open(&loop);
// 	uv_tcp_t * handle = reinterpret_cast<uv_tcp_t *>(impl->GetHandle());
// #ifdef _WIN32
// 	handle->socket = 10000;
// #else
// 	handle->io_watcher.fd = 10000;
// #endif
// 	impl->ShutdownReceive();
// 	impl->SetNoDelay();
// 	impl->SetKeepAlive(60);
// 	impl->Listen(128, nullptr);
// 	impl->Release();
// 	uv_run(&loop, UV_RUN_DEFAULT);
// 	uv_loop_close(&loop);
// }

// TEST(SocketTestSuite, FreeCatch) {
// 	uv_udp_t * handle = new uv_udp_t();
// 	try {
// 		handle->type = UV_UDP;
// 		SocketImpl::FreeHandle(reinterpret_cast<uv_handle_t *>(handle));
// 	} catch (std::exception & e) {
// 		printf("SocketTestSuite - FreeCatch: %s\n", e.what());
// 	}
// 	delete handle;
// }