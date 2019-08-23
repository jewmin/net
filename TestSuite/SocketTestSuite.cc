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
	free(handle);
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
	buf->base = static_cast<char *>(malloc(suggested_size));
	buf->len = suggested_size;
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
			int out = client.GetSendBufferSize();
			int in = client.GetReceiveBufferSize();
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
		int out = client.GetSendBufferSize();
		int in = client.GetReceiveBufferSize();
		client.SetSendBufferSize(1024);
		client.SetReceiveBufferSize(1024);
		uv_write_t * req = new uv_write_t();
		client.Send("this is a test message\0", sizeof("this is a test message\0"), req, write_cb);
	}
}

TEST(SocketTestSuite, Operation) {
	ServerSocket server;
	server.Open(uv_default_loop());
	server.Bind(SocketAddress("0.0.0.0", 6789));
	server.Listen(128, connection_cb);
	Context * server_context = new Context();
	server_context->server = server;
	uv_handle_set_data(server.GetHandle(), server_context);

	StreamSocket client;
	client.Open(uv_default_loop());
	uv_connect_t * req = new uv_connect_t();
	client.Connect(SocketAddress("127.0.0.1", 6789), req, connect_cb);
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

	s3.Bind(SocketAddress("127.0.0.1", 6789), true, true);
	s3.Bind(SocketAddress("::1", 6789), true, true);
	other.Close();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}