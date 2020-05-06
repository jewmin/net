#include "gtest/gtest.h"
#include "Sockets/Socket.h"
#include "Sockets/StreamSocket.h"
#include "Sockets/ServerSocket.h"
#include "Sockets/StreamSocketImpl.h"
#include "Sockets/ServerSocketImpl.h"
#include "Common/Allocator.h"

class MockSocket : public Net::Socket {
public:
	MockSocket(Net::SocketImpl * impl) : Net::Socket(impl) {}
};

class SocketTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		socket_ = new MockSocket(new Net::StreamSocketImpl());
		EXPECT_EQ(socket_->Impl()->ReferenceCount(), 1);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete socket_;
	}

	Net::Socket * socket_;
};

TEST_F(SocketTestSuite, ctor) {
	Net::Socket so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
}

TEST_F(SocketTestSuite, ctor2) {
	EXPECT_ANY_THROW(MockSocket so(nullptr));
}

TEST_F(SocketTestSuite, ctor3) {
	{
		Net::Socket so(*socket_);
		EXPECT_EQ(socket_->Impl()->ReferenceCount(), 2);
	}
	EXPECT_EQ(socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketTestSuite, assign) {
	Net::Socket so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
	so = so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
}

TEST_F(SocketTestSuite, assign2) {
	Net::Socket so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
	so = *socket_;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	so = *socket_;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
}

TEST_F(SocketTestSuite, assign3) {
	*socket_ = Net::Socket();
	EXPECT_EQ(socket_->Impl()->ReferenceCount(), 1);
	*socket_ = *socket_;
	EXPECT_EQ(socket_->Impl()->ReferenceCount(), 1);
}

class SocketLoopTestSuite : public SocketTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		loop_ = static_cast<uv_loop_t *>(jc_malloc(sizeof(*loop_)));
		uv_loop_init(loop_);
		SocketTestSuite::SetUp();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		SocketTestSuite::TearDown();
		uv_run(loop_, UV_RUN_DEFAULT);
		uv_loop_close(loop_);
		jc_free(loop_);
	}

	void Loop(i32 count = 10) {
		while (count-- > 0) {
			uv_run(loop_, UV_RUN_NOWAIT);
		}
	}

	uv_loop_t * loop_;
};

TEST_F(SocketLoopTestSuite, open) {
	socket_->Open(loop_);
	socket_->Open(loop_);
	socket_->Close();
}

class MockUvData : public Net::UvData {
public:
	virtual ~MockUvData() {}
};

class SocketOpenTestSuite : public SocketLoopTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketLoopTestSuite::SetUp();
		socket_->Open(loop_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		socket_->Close();
		SocketLoopTestSuite::TearDown();
	}
};

TEST_F(SocketOpenTestSuite, operation) {
	socket_->SetSendBufferSize(1024);
	socket_->SetRecvBufferSize(1024);
	EXPECT_EQ(socket_->GetSendBufferSize(), 0);
	EXPECT_EQ(socket_->GetRecvBufferSize(), 0);
	EXPECT_EQ(socket_->GetWriteQueueSize(), 0);
	EXPECT_EQ(socket_->LocalAddress(), Net::SocketAddress());
	EXPECT_EQ(socket_->RemoteAddress(), Net::SocketAddress());
	socket_->SetNoDelay();
	socket_->SetKeepAlive(60);
	Net::UvData * data = new MockUvData();
	socket_->SetUvData(data);
	socket_->SetUvData(nullptr);
	data->Destroy();
}

class SocketCmpTestSuite : public SocketOpenTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketOpenTestSuite::SetUp();
		for (i32 i = 0; i < 3; ++i) {
			impl_[i].Duplicate();
		}
		small_socket_ = new MockSocket(&impl_[0]);
		equals_socket_ = new MockSocket(&impl_[1]);
		big_socket_ = new MockSocket(&impl_[2]);
		*socket_ = *equals_socket_;
		EXPECT_EQ(small_socket_->Impl()->ReferenceCount(), 2);
		EXPECT_EQ(equals_socket_->Impl()->ReferenceCount(), 3);
		EXPECT_EQ(big_socket_->Impl()->ReferenceCount(), 2);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete big_socket_;
		delete equals_socket_;
		delete small_socket_;
		EXPECT_EQ(impl_[0].ReferenceCount(), 1);
		EXPECT_EQ(impl_[1].ReferenceCount(), 2);
		EXPECT_EQ(impl_[2].ReferenceCount(), 1);
		SocketOpenTestSuite::TearDown();
		EXPECT_EQ(impl_[0].ReferenceCount(), 1);
		EXPECT_EQ(impl_[1].ReferenceCount(), 1);
		EXPECT_EQ(impl_[2].ReferenceCount(), 1);
	}

	Net::StreamSocketImpl impl_[3];
	Net::Socket * big_socket_;
	Net::Socket * equals_socket_;
	Net::Socket * small_socket_;
};

TEST_F(SocketCmpTestSuite, cmp) {
	EXPECT_EQ(*small_socket_, *small_socket_);
	EXPECT_EQ(*equals_socket_, *equals_socket_);
	EXPECT_EQ(*big_socket_, *big_socket_);
	EXPECT_EQ(*equals_socket_, *socket_);
}

TEST_F(SocketCmpTestSuite, cmp1) {
	EXPECT_NE(*small_socket_, *equals_socket_);
	EXPECT_NE(*small_socket_, *big_socket_);
	EXPECT_NE(*equals_socket_, *big_socket_);
}

TEST_F(SocketCmpTestSuite, cmp2) {
	EXPECT_LT(*small_socket_, *equals_socket_);
	EXPECT_LT(*small_socket_, *big_socket_);
	EXPECT_LT(*equals_socket_, *big_socket_);
}

TEST_F(SocketCmpTestSuite, cmp3) {
	EXPECT_LE(*small_socket_, *small_socket_);
	EXPECT_LE(*small_socket_, *equals_socket_);
	EXPECT_LE(*small_socket_, *big_socket_);
	EXPECT_LE(*equals_socket_, *equals_socket_);
	EXPECT_LE(*equals_socket_, *big_socket_);
	EXPECT_LE(*big_socket_, *big_socket_);
}

TEST_F(SocketCmpTestSuite, cmp4) {
	EXPECT_GT(*equals_socket_, *small_socket_);
	EXPECT_GT(*big_socket_, *small_socket_);
	EXPECT_GT(*big_socket_, *equals_socket_);
}

TEST_F(SocketCmpTestSuite, cmp5) {
	EXPECT_GE(*small_socket_, *small_socket_);
	EXPECT_GE(*equals_socket_, *small_socket_);
	EXPECT_GE(*big_socket_, *small_socket_);
	EXPECT_GE(*equals_socket_, *equals_socket_);
	EXPECT_GE(*big_socket_, *equals_socket_);
	EXPECT_GE(*big_socket_, *big_socket_);
}

class SocketServerTestSuite : public SocketOpenTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketOpenTestSuite::SetUp();
		server_socket_ = new Net::ServerSocket();
		other_server_socket_ = new Net::ServerSocket();
		EXPECT_EQ(server_socket_->Impl()->ReferenceCount(), 1);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete server_socket_;
		delete other_server_socket_;
		SocketOpenTestSuite::TearDown();
	}

	Net::ServerSocket * server_socket_;
	Net::Socket * other_server_socket_;
};

TEST_F(SocketServerTestSuite, ctor) {
	{
		Net::ServerSocket so(*server_socket_);
		EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	}
	EXPECT_EQ(server_socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketServerTestSuite, ctor2) {
	EXPECT_ANY_THROW(Net::ServerSocket so(*socket_));
}

TEST_F(SocketServerTestSuite, ctor3) {
	{
		Net::ServerSocket so(*other_server_socket_);
		EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	}
	EXPECT_EQ(other_server_socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketServerTestSuite, assign) {
	*server_socket_ = Net::ServerSocket();
	EXPECT_EQ(server_socket_->Impl()->ReferenceCount(), 1);
	*server_socket_ = *server_socket_;
	EXPECT_EQ(server_socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketServerTestSuite, assign2) {
	Net::ServerSocket so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
	so = *server_socket_;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	so = so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	so = *other_server_socket_;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
}

TEST_F(SocketServerTestSuite, assign3) {
	Net::ServerSocket so;
	EXPECT_ANY_THROW(so = *socket_);
}

class SocketServerOpenTestSuite : public SocketServerTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketServerTestSuite::SetUp();
		server_address_ = Net::SocketAddress(Net::IPAddress("::"), 6789);
		server_socket_->Open(loop_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		server_socket_->Close();
		SocketServerTestSuite::TearDown();
	}

	Net::SocketAddress server_address_;
};

TEST_F(SocketServerOpenTestSuite, bind) {
	EXPECT_EQ(server_socket_->Bind(server_address_, true, true), 0);
}

TEST_F(SocketServerOpenTestSuite, listen) {
	EXPECT_EQ(server_socket_->Listen(), 0);
}

TEST_F(SocketServerOpenTestSuite, accept) {
	Net::StreamSocket so;
	EXPECT_EQ(server_socket_->AcceptSocket(so), false);
}

class MockStreamSocket : public Net::StreamSocket {
public:
	MockStreamSocket(Net::SocketImpl * impl) : Net::StreamSocket(impl) {}
};

class SocketStreamTestSuite : public SocketServerOpenTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketServerOpenTestSuite::SetUp();
		stream_socket_ = new Net::StreamSocket();
		other_stream_socket_ = new Net::StreamSocket();
		EXPECT_EQ(stream_socket_->Impl()->ReferenceCount(), 1);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete stream_socket_;
		delete other_stream_socket_;
		SocketServerOpenTestSuite::TearDown();
	}

	Net::StreamSocket * stream_socket_;
	Net::Socket * other_stream_socket_;
};

TEST_F(SocketStreamTestSuite, ctor) {
	{
		Net::StreamSocket so(*stream_socket_);
		EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	}
	EXPECT_EQ(stream_socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketStreamTestSuite, ctor2) {
	EXPECT_ANY_THROW(Net::StreamSocket so(*server_socket_));
}

TEST_F(SocketStreamTestSuite, ctor3) {
	MockStreamSocket so(new Net::StreamSocketImpl());
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
}

TEST_F(SocketStreamTestSuite, ctor4) {
	EXPECT_ANY_THROW(MockStreamSocket so(new Net::ServerSocketImpl()));
}

TEST_F(SocketStreamTestSuite, ctor5) {
	{
		Net::StreamSocket so(*other_stream_socket_);
		EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	}
	EXPECT_EQ(other_stream_socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketStreamTestSuite, assign) {
	*stream_socket_ = Net::StreamSocket();
	EXPECT_EQ(stream_socket_->Impl()->ReferenceCount(), 1);
	*stream_socket_ = *stream_socket_;
	EXPECT_EQ(stream_socket_->Impl()->ReferenceCount(), 1);
}

TEST_F(SocketStreamTestSuite, assign2) {
	Net::StreamSocket so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 1);
	so = *stream_socket_;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	so = so;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
	so = *other_stream_socket_;
	EXPECT_EQ(so.Impl()->ReferenceCount(), 2);
}

TEST_F(SocketStreamTestSuite, assign3) {
	Net::StreamSocket so;
	EXPECT_ANY_THROW(so = *server_socket_);
}

class SocketStreamOpenTestSuite : public SocketStreamTestSuite {
public:
	SocketStreamOpenTestSuite() {
		std::memset(w_content_, 0, sizeof(w_content_));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketStreamTestSuite::SetUp();
		std::strncpy(w_content_, "hello world", std::strlen("hello world"));
		stream_address_ = Net::SocketAddress(Net::IPAddress("::1"), 6789);
		stream_socket_->Open(loop_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		stream_socket_->Close();
		SocketStreamTestSuite::TearDown();
	}

	i8 w_content_[12];
	Net::SocketAddress stream_address_;
};

TEST_F(SocketStreamOpenTestSuite, bind) {
	EXPECT_EQ(stream_socket_->Bind(stream_address_, true, true), 0);
}

TEST_F(SocketStreamOpenTestSuite, connect) {
	EXPECT_EQ(stream_socket_->Connect(stream_address_), 0);
}

TEST_F(SocketStreamOpenTestSuite, shutdown) {
	EXPECT_EQ(stream_socket_->ShutdownRead(), 0);
	EXPECT_EQ(stream_socket_->ShutdownWrite(), UV_ENOTCONN);
	EXPECT_EQ(stream_socket_->Shutdown(), UV_ENOTCONN);
}

TEST_F(SocketStreamOpenTestSuite, established) {
	EXPECT_EQ(stream_socket_->Established(), UV_ENOTCONN);
}

TEST_F(SocketStreamOpenTestSuite, write) {
	EXPECT_EQ(stream_socket_->Bind(stream_address_, true, true), 0);
	EXPECT_EQ(stream_socket_->Write(w_content_, (i32)std::strlen(w_content_)), UV_EPIPE);
}

class SocketStreamServerTestSuite : public SocketStreamOpenTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketStreamOpenTestSuite::SetUp();
		EXPECT_EQ(server_socket_->Bind(server_address_, true, true), 0);
		EXPECT_EQ(server_socket_->Listen(), 0);
		EXPECT_EQ(stream_socket_->Connect(stream_address_), 0);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		SocketStreamOpenTestSuite::TearDown();
	}
};

TEST_F(SocketStreamServerTestSuite, accept) {
	Loop(30);
	Net::StreamSocket so;
	EXPECT_EQ(server_socket_->AcceptSocket(so), true);
}