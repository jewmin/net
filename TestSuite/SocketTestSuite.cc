#include "gtest/gtest.h"
#include "Sockets/Socket.h"
#include "Sockets/ServerSocket.h"
#include "Sockets/StreamSocket.h"
#include "Sockets/ServerSocketImpl.h"

using namespace Net;

TEST(SocketTestSuite, Construct) {
	Socket s1;
	Socket s2(s1);
	Socket s3;
	Socket s4;
	s3 = s1;
}

TEST(SocketTestSuite, Equal) {
	Socket sockets[10];
	std::set<Socket> socketSet;
	for (int i = 0; i < 10; ++i) {
		socketSet.insert(Socket());
	}
	int i = 0;
	for (std::set<Socket>::iterator it = socketSet.begin(); it != socketSet.end(); ++it) {
		sockets[i++] = *it;
	}
	EXPECT_GT(sockets[1], sockets[0]);
	EXPECT_GE(sockets[3], sockets[2]);
	EXPECT_LT(sockets[4], sockets[5]);
	EXPECT_LE(sockets[6], sockets[7]);
	EXPECT_EQ(sockets[8], sockets[8]);
	EXPECT_NE(sockets[9], sockets[0]);
}

struct Context {
	ServerSocket server;
	StreamSocket client;
};

void close_cb(uv_handle_t* handle) {
	Context * context = static_cast<Context *>(handle->data);
	SocketImpl::FreeHandle(handle);
	if (context) {
		delete context;
	}
}

void shutdown_cb(uv_shutdown_t* req, int status) {
	Context * context = static_cast<Context *>(req->handle->data);
	delete req;
	if (context) {
		context->client.Close(close_cb);
	}
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	buf->base = static_cast<char *>(malloc(64 * 1024));
	buf->len = 64 * 1024;
}

void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
	Context * context = static_cast<Context *>(stream->data);
	if (nread > 0) {
		printf("%s\n", buf->base);
	}
	if (buf->base) {
		free(buf->base);
	}
	if (context) {
		uv_shutdown_t * shutdown_req = new uv_shutdown_t();
		context->client.ShutdownReceive();
		context->client.ShutdownSend(shutdown_req, shutdown_cb);
		context->server.Close(close_cb);
	}
}

void write_cb(uv_write_t* req, int status) {
	Context * context = static_cast<Context *>(req->handle->data);
	delete req;
	if (context) {
		StreamSocket client = context->client;
		uv_shutdown_t * shutdown_req = new uv_shutdown_t();
		client.Shutdown(shutdown_req, shutdown_cb);
	}
}

void connection_cb(uv_stream_t* server, int status) {
	Context * context = static_cast<Context *>(server->data);
	ServerSocket srv = context->server;
	if (status < 0) {
		srv.Close(close_cb);
	} else {
		StreamSocket client;
		if (srv.AcceptConnection(client)) {
			context->client = client;
			client.SetNoDelay();
			client.SetKeepAlive(60);
			client.SetSendBufferSize(1024);
			client.SetReceiveBufferSize(1024);
			client.GetSendBufferSize();
			client.GetReceiveBufferSize();
			SocketAddress remote = client.RemoteAddress();
			client.Established(alloc_cb, read_cb);
			Context * client_context = new Context();
			client_context->server = srv;
			client_context->client = client;
			uv_handle_set_data(client.GetHandle(), client_context);
		} else {
			srv.Close(close_cb);
		}
	}
}

void connect_cb(uv_connect_t* req, int status) {
	Context * context = static_cast<Context *>(req->handle->data);
	ServerSocket server = context->server;
	StreamSocket client = context->client;
	delete req;
	if (status < 0) {
		server.Close(close_cb);
		client.Close(close_cb);
	} else {
		client.SetNoDelay();
		client.SetKeepAlive(60);
		SocketAddress local = client.LocalAddress();
		client.GetSendBufferSize();
		client.GetReceiveBufferSize();
		client.SetSendBufferSize(1024);
		client.SetReceiveBufferSize(1024);
		uv_write_t * req = new uv_write_t();
		client.Send("this is a test message", sizeof("this is a test message"), req, write_cb);
	}
}

TEST(SocketTestSuite, Operation) {
	ServerSocket server;
	server.Open(uv_default_loop());
	server.Bind(SocketAddress(IPAddress("0.0.0.0"), 6789));
	server.Listen(128, connection_cb);
	Context * server_context = new Context();
	server_context->server = server;
	uv_handle_set_data(server.GetHandle(), server_context);

	StreamSocket client;
	client.Open(uv_default_loop());
	uv_connect_t * req = new uv_connect_t();
	client.Connect(SocketAddress(IPAddress("127.0.0.1"), 6789), req, connect_cb);
	Context * client_context = new Context();
	client_context->server = server;
	client_context->client = client;
	uv_handle_set_data(client.GetHandle(), client_context);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(SocketTestSuite, Assign) {
	Socket socket, other;
	socket.Open(uv_default_loop());
	uv_handle_t * handle = socket.GetHandle();
	other.Attach(socket.Detatch());
	EXPECT_EQ(other.GetHandle(), handle);
	EXPECT_EQ(socket.GetHandle(), nullptr);

	StreamSocket s1;
	StreamSocket s2(socket);
	StreamSocket s3;
	s3 = other;

	ServerSocket t1;
	socket = t1;
	ServerSocket t2(socket);
	ServerSocket t3;
	t3 = socket;

	s3.Bind(SocketAddress(IPAddress("127.0.0.1"), 6789), true, true);
	s3.Bind(SocketAddress(IPAddress("::1"), 6789), true, true);
	other.Close();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

class MockSocketImpl : public SocketImpl {
public:
	MockSocketImpl() : SocketImpl() {}
	virtual ~MockSocketImpl() {}
};

class MockSocket : public Socket {
public:
	MockSocket() : Socket(new MockSocketImpl()) {}
	virtual ~MockSocket() {}
};

class MockStreamSocket : public StreamSocket {
public:
	MockStreamSocket(SocketImpl * impl) : StreamSocket(impl) {}
	virtual ~MockStreamSocket() {}
};

TEST(SocketTestSuite, Catch) {
	MockSocket ms;
	try {
		ServerSocket s1(ms);
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		ServerSocket s2;
		s2 = ms;
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		StreamSocket s3(ms);
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		StreamSocket s4;
		s4 = ms;
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}

	try {
		MockStreamSocket s5(new ServerSocketImpl());
	} catch (std::exception & e) {
		printf("SocketTestSuite - Catch: %s\n", e.what());
	}
}

TEST(SocketTestSuite, ImplCatch) {
	MockSocketImpl * impl = new MockSocketImpl();

	try {
		impl->Open(uv_default_loop());
		impl->Attach(nullptr);
	} catch (std::exception & e) {
		printf("SocketTestSuite - ImplCatch: %s\n", e.what());
	}

	impl->Established(nullptr, nullptr);
	impl->LocalAddress();
	impl->RemoteAddress();
	impl->GetSendBufferSize();
	impl->GetReceiveBufferSize();
	impl->SetSendBufferSize(10);
	impl->SetReceiveBufferSize(10);
	impl->Send(nullptr, 0, nullptr, nullptr);
	uv_write_t * w_req = new uv_write_t();
	impl->Send("hello", sizeof("hello"), w_req, nullptr);
	delete w_req;
	uv_shutdown_t * req = new uv_shutdown_t();
	impl->ShutdownSend(req, nullptr);
	delete req;
	impl->Close();
	impl->Send(nullptr, 0, nullptr, nullptr);

	impl->Release();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(SocketTestSuite, AcceptError) {
	ServerSocket socket;
	socket.Open(uv_default_loop());
	StreamSocket client;
	EXPECT_EQ(socket.AcceptConnection(client), false);
	socket.Close();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

void SocketTestSuite_connect_cb(uv_connect_t* req, int status) {
	delete req;
}

void SocketTestSuite_listen_cb(uv_stream_t* server, int status) {
	
}

TEST(SocketTestSuite, ConnectError) {
	StreamSocket client;
	client.Open(uv_default_loop());
	uv_connect_t * req1 = new uv_connect_t();
	uv_connect_t * req2 = new uv_connect_t();
	client.Connect(SocketAddress(IPAddress("127.0.0.1"), 6789), req1, SocketTestSuite_connect_cb);
	client.Connect(SocketAddress(IPAddress("127.0.0.1"), 6789), req2, SocketTestSuite_connect_cb);
	delete req2;
	client.Close();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(SocketTestSuite, handleError) {
	MockSocketImpl * impl = new MockSocketImpl();
	uv_tcp_t handle;
	uv_tcp_init(uv_default_loop(), &handle);
#ifdef _WIN32
	handle.socket = 10000;
#else
	handle.io_watcher.fd = 10000;
#endif
	impl->Attach(reinterpret_cast<uv_handle_t *>(&handle));
	impl->Listen(128, nullptr);
	impl->ShutdownReceive();
	impl->SetNoDelay();
	impl->SetKeepAlive(60);
	impl->Release();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(SocketTestSuite, FreeCatch) {
	uv_udp_t * handle = new uv_udp_t();
	try {
		handle->type = UV_UDP;
		SocketImpl::FreeHandle(reinterpret_cast<uv_handle_t *>(handle));
	} catch (std::exception & e) {
		printf("SocketTestSuite - FreeCatch: %s\n", e.what());
	}
	delete handle;
}