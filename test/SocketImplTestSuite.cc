#include "gtest/gtest.h"
#include "Common/Allocator.h"
#include "Sockets/SocketImpl.h"

class MockSocketImpl : public Net::SocketImpl {
public:
	void delayed_error(i32 err) {
		reinterpret_cast<uv_tcp_t *>(handle_)->delayed_error = err;
	}
	void write_queue_size(i32 size) {
		reinterpret_cast<uv_tcp_t *>(handle_)->write_queue_size = size;
	}
#ifdef _WIN32
	void io_fd(SOCKET fd) {
		fd_ = reinterpret_cast<uv_tcp_t *>(handle_)->socket;
		reinterpret_cast<uv_tcp_t *>(handle_)->socket = fd;
	}
	SOCKET fd_;
#else
	void io_fd(i32 fd) {
		fd_ = reinterpret_cast<uv_stream_t *>(handle_)->io_watcher.fd;
		reinterpret_cast<uv_stream_t *>(handle_)->io_watcher.fd = fd;
	}
	i32 fd_;
#endif
	void OpenPipe(uv_loop_t * loop) {
		if (!handle_) {
			handle_ = static_cast<uv_handle_t *>(jc_malloc(sizeof(uv_pipe_t)));
			uv_pipe_init(loop, reinterpret_cast<uv_pipe_t *>(handle_), 0);
			handle_->data = nullptr;
		}
	}
	uv_handle_t * GetHandle() { return handle_; }
};

class MockUvData : public Net::UvData {
};

class UvDataTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		uv_data_ = new MockUvData();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete uv_data_;
	}

	Net::UvData * uv_data_;
};

class SocketImplTestSuite : public UvDataTestSuite {
public:
	SocketImplTestSuite() {
		std::memset(w_content_, 0, sizeof(w_content_));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		UvDataTestSuite::SetUp();
		std::memcpy(w_content_, "hello world", std::strlen("hello world"));
		loop_ = static_cast<uv_loop_t *>(jc_malloc(sizeof(*loop_)));
		uv_loop_init(loop_);
		socket_impl_ = new MockSocketImpl();
		address_ipv6_any_ = Net::SocketAddress("::", 0);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		socket_impl_->Release();
		uv_run(loop_, UV_RUN_DEFAULT);
		uv_loop_close(loop_);
		jc_free(loop_);
		UvDataTestSuite::TearDown();
	}
	
	i8 w_content_[12];
	uv_loop_t * loop_;
	MockSocketImpl * socket_impl_;
	Net::SocketAddress address_ipv6_any_;
	Net::SocketAddress address_ipv4_any_;
};

TEST_F(SocketImplTestSuite, open) {
	socket_impl_->Open(loop_);
	socket_impl_->Open(loop_);
	socket_impl_->Close();
	socket_impl_->Close();
}

TEST_F(SocketImplTestSuite, close) {
	socket_impl_->OpenPipe(loop_);
	uv_handle_t * handle = socket_impl_->GetHandle();
	socket_impl_->Close();
	EXPECT_ANY_THROW(uv_run(loop_, UV_RUN_DEFAULT));
	jc_free(handle);
}

TEST_F(SocketImplTestSuite, bind) {
	EXPECT_EQ(socket_impl_->Bind(address_ipv6_any_), UV_UNKNOWN);
}

TEST_F(SocketImplTestSuite, listen) {
	EXPECT_EQ(socket_impl_->Listen(), UV_UNKNOWN);
}

TEST_F(SocketImplTestSuite, connect) {
	EXPECT_EQ(socket_impl_->Connect(address_ipv6_any_), UV_UNKNOWN);
}

TEST_F(SocketImplTestSuite, accept) {
	Net::SocketAddress so;
	EXPECT_TRUE(socket_impl_->AcceptSocket(so) == nullptr);
}

TEST_F(SocketImplTestSuite, shutdown) {
	EXPECT_EQ(socket_impl_->ShutdownRead(), UV_UNKNOWN);
	EXPECT_EQ(socket_impl_->ShutdownWrite(), UV_UNKNOWN);
	EXPECT_EQ(socket_impl_->Shutdown(), UV_UNKNOWN);
}

TEST_F(SocketImplTestSuite, established) {
	EXPECT_EQ(socket_impl_->Established(), UV_UNKNOWN);
}

TEST_F(SocketImplTestSuite, write) {
	EXPECT_EQ(socket_impl_->Write(nullptr, 0), UV_EPROTONOSUPPORT);
}

TEST_F(SocketImplTestSuite, buff) {
	socket_impl_->SetSendBufferSize(1024);
	socket_impl_->SetRecvBufferSize(1024);
	EXPECT_EQ(socket_impl_->GetSendBufferSize(), 0);
	EXPECT_EQ(socket_impl_->GetRecvBufferSize(), 0);
	EXPECT_EQ(socket_impl_->GetWriteQueueSize(), 0);
}

TEST_F(SocketImplTestSuite, address) {
	EXPECT_EQ(socket_impl_->LocalAddress(), Net::SocketAddress());
	EXPECT_EQ(socket_impl_->RemoteAddress(), Net::SocketAddress());
}

TEST_F(SocketImplTestSuite, opt) {
	socket_impl_->SetNoDelay();
	socket_impl_->SetKeepAlive(60);
	socket_impl_->SetUvData(uv_data_);
	socket_impl_->SetUvData(nullptr);
}

class SocketImplOpenTestSuite : public SocketImplTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketImplTestSuite::SetUp();
		socket_impl_->Open(loop_);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		socket_impl_->Close();
		SocketImplTestSuite::TearDown();
	}
};

TEST_F(SocketImplOpenTestSuite, bind) {
	EXPECT_EQ(socket_impl_->Bind(address_ipv4_any_), 0);
}

TEST_F(SocketImplOpenTestSuite, bind2) {
	EXPECT_LT(socket_impl_->Bind(address_ipv4_any_, true), 0);
}

TEST_F(SocketImplOpenTestSuite, listen) {
	EXPECT_EQ(socket_impl_->Listen(), 0);
}

TEST_F(SocketImplOpenTestSuite, listen2) {
	EXPECT_EQ(socket_impl_->Bind(address_ipv6_any_), 0);
	EXPECT_EQ(socket_impl_->Listen(5), 0);
}

TEST_F(SocketImplOpenTestSuite, listen3) {
	socket_impl_->delayed_error(UV_ECANCELED);
	EXPECT_EQ(socket_impl_->Listen(), UV_ECANCELED);
}

TEST_F(SocketImplOpenTestSuite, connect) {
	EXPECT_EQ(socket_impl_->Connect(Net::SocketAddress("127.0.0.1", 6789)), 0);
}

TEST_F(SocketImplOpenTestSuite, connect2) {
#ifdef _WIN32
	socket_impl_->delayed_error(UV_EALREADY);
#else
	EXPECT_EQ(socket_impl_->Connect(Net::SocketAddress("127.0.0.1", 6789)), 0);
#endif
	EXPECT_EQ(socket_impl_->Connect(Net::SocketAddress("127.0.0.1", 6789)), UV_EALREADY);
}

TEST_F(SocketImplOpenTestSuite, accept) {
	Net::SocketAddress so;
	EXPECT_TRUE(socket_impl_->AcceptSocket(so) == nullptr);
}

TEST_F(SocketImplOpenTestSuite, shutdown) {
	EXPECT_EQ(socket_impl_->ShutdownRead(), 0);
	EXPECT_EQ(socket_impl_->ShutdownWrite(), UV_ENOTCONN);
	EXPECT_EQ(socket_impl_->Shutdown(), UV_ENOTCONN);
}

TEST_F(SocketImplOpenTestSuite, established) {
	EXPECT_EQ(socket_impl_->Established(), UV_ENOTCONN);
}

TEST_F(SocketImplOpenTestSuite, write) {
	EXPECT_EQ(socket_impl_->Bind(address_ipv6_any_), 0);
	EXPECT_EQ(socket_impl_->Write(nullptr, 1), UV_ENOBUFS);
	EXPECT_EQ(socket_impl_->Write(w_content_, 0), UV_ENOBUFS);
	EXPECT_EQ(socket_impl_->Write(w_content_, (i32)std::strlen(w_content_)), UV_EPIPE);
}

TEST_F(SocketImplOpenTestSuite, buff) {
	socket_impl_->SetSendBufferSize(1024);
	socket_impl_->SetRecvBufferSize(1024);
	EXPECT_EQ(socket_impl_->GetSendBufferSize(), 0);
	EXPECT_EQ(socket_impl_->GetRecvBufferSize(), 0);
	socket_impl_->write_queue_size(100);
	EXPECT_EQ(socket_impl_->GetWriteQueueSize(), 100);
	socket_impl_->write_queue_size(0);
}

TEST_F(SocketImplOpenTestSuite, address) {
	EXPECT_EQ(socket_impl_->LocalAddress(), Net::SocketAddress());
	EXPECT_EQ(socket_impl_->RemoteAddress(), Net::SocketAddress());
}

TEST_F(SocketImplOpenTestSuite, opt) {
	socket_impl_->io_fd(0);
	socket_impl_->SetNoDelay();
	socket_impl_->SetKeepAlive(60);
	socket_impl_->io_fd(socket_impl_->fd_);
	socket_impl_->SetUvData(uv_data_);
	socket_impl_->SetUvData(uv_data_);
	socket_impl_->SetUvData(nullptr);
}

class MockUvData2 : public MockUvData {
public:
	MockUvData2()
		: call_close_count_(0), call_accpet_count_(0), call_connect_count_(0), call_shutdown_count_(0)
		, call_alloc_count_(0), call_read_count_(0), call_written_count_(0), server_impl_(nullptr), client_impl_(nullptr) {}
	virtual ~MockUvData2() {
		if (server_impl_) server_impl_->Release();
		if (client_impl_) client_impl_->Release();
	}
	virtual void CloseCallback() {
		MockUvData::CloseCallback();
		call_close_count_++;
	}
	virtual void AcceptCallback(i32 status) {
		MockUvData::AcceptCallback(status);
		call_accpet_count_++;
		if (status < 0) {
			std::printf("MockUvData2::AcceptCallback: %s\n", uv_strerror(status));
		} else if (server_impl_) {
			Net::SocketAddress client_address;
			client_impl_ = server_impl_->AcceptSocket(client_address);
			EXPECT_TRUE(client_impl_ != nullptr);
			client_impl_->SetUvData(this);
		}
	}
	virtual void ConnectCallback(i32 status, void * arg) {
		MockUvData::ConnectCallback(status, arg);
		call_connect_count_++;
		if (status < 0) {
			std::printf("MockUvData2::ConnectCallback: %s\n", uv_strerror(status));
		}
	}
	virtual void ShutdownCallback(i32 status, void * arg) {
		MockUvData::ShutdownCallback(status, arg);
		call_shutdown_count_++;
		if (status < 0) {
			std::printf("MockUvData2::ShutdownCallback: %s\n", uv_strerror(status));
		}
	}
	virtual void AllocCallback(uv_buf_t * buf) {
		MockUvData::AllocCallback(buf);
		call_alloc_count_++;
	}
	virtual void ReadCallback(i32 status) {
		MockUvData::ReadCallback(status);
		call_read_count_++;
		if (status < 0) {
			std::printf("MockUvData2::ReadCallback: %s\n", uv_strerror(status));
		}
	}
	virtual void WrittenCallback(i32 status, void * arg) {
		MockUvData::WrittenCallback(status, arg);
		call_written_count_++;
		if (status < 0) {
			std::printf("MockUvData2::WrittenCallback: %s\n", uv_strerror(status));
		}
	}
	void SetServer(Net::SocketImpl * impl) {
		server_impl_ = impl;
		server_impl_->Duplicate();
	}
	void SetClient(Net::SocketImpl * impl) {
		client_impl_ = impl;
		client_impl_->Duplicate();
	}

	i32 call_close_count_;
	i32 call_accpet_count_;
	i32 call_connect_count_;
	i32 call_shutdown_count_;
	i32 call_alloc_count_;
	i32 call_read_count_;
	i32 call_written_count_;
	Net::SocketImpl * server_impl_;
	Net::SocketImpl * client_impl_;
};

class SocketImplCbNullTestSuite : public SocketImplOpenTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketImplOpenTestSuite::SetUp();
		server_data_ = new MockUvData2();
		client_data_ = new MockUvData2();
		client_socket_impl_ = new MockSocketImpl();
		Listen();
		Connect();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		client_socket_impl_->Release();
		delete client_data_;
		delete server_data_;
		SocketImplOpenTestSuite::TearDown();
	}

	void Connect() {
		client_socket_impl_->Open(loop_);
		client_socket_impl_->Connect(address_);
	}

	void Listen() {
		socket_impl_->Bind(address_ipv6_any_);
		socket_impl_->Listen();
		address_ = Net::SocketAddress("127.0.0.1", socket_impl_->LocalAddress().Port());
	}

	void Loop(i32 count = 30) {
		while (count-- > 0) {
			uv_run(loop_, UV_RUN_NOWAIT);
		}
	}

	void SetUvData() {
		socket_impl_->SetUvData(server_data_);
		server_data_->SetServer(socket_impl_);
		client_socket_impl_->SetUvData(client_data_);
		client_data_->SetClient(client_socket_impl_);
	}

	MockSocketImpl * client_socket_impl_;
	MockUvData2 * server_data_;
	MockUvData2 * client_data_;
	Net::SocketAddress address_;
};

TEST_F(SocketImplCbNullTestSuite, cb) {
	Loop();
}

TEST_F(SocketImplCbNullTestSuite, cb2) {
	SetUvData();
	Loop();
	EXPECT_EQ(server_data_->call_accpet_count_, 1);
	EXPECT_EQ(client_data_->call_connect_count_, 1);
	client_socket_impl_->Shutdown();
	client_socket_impl_->Close();
	Loop();
	EXPECT_EQ(client_data_->call_shutdown_count_, 1);
	EXPECT_EQ(client_data_->call_close_count_, 1);
}

TEST_F(SocketImplCbNullTestSuite, cb3) {
	SetUvData();
	Loop();
	EXPECT_GE(server_data_->call_accpet_count_, 0);
	EXPECT_GE(client_data_->call_connect_count_, 0);
	EXPECT_EQ(server_data_->client_impl_->Established(), 0);
	client_socket_impl_->Write("123", 3);
	Loop();
	EXPECT_EQ(client_data_->call_written_count_, 1);
	EXPECT_GE(server_data_->call_alloc_count_, 1);
	EXPECT_GE(server_data_->call_read_count_, 1);
}

TEST_F(SocketImplCbNullTestSuite, cb4) {
	MockUvData * data = new MockUvData();
	socket_impl_->SetUvData(data);
	client_socket_impl_->SetUvData(data);
	delete data;
	Loop();
}

class SocketImplCbTestSuite : public SocketImplCbNullTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketImplCbNullTestSuite::SetUp();
		SetUvData();
		Loop();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		SocketImplCbNullTestSuite::TearDown();
	}
};

TEST_F(SocketImplCbTestSuite, shutdown) {
	EXPECT_EQ(client_socket_impl_->ShutdownWrite(), 0);
}

TEST_F(SocketImplCbTestSuite, shutdown2) {
	client_socket_impl_->SetUvData(nullptr);
	EXPECT_EQ(client_socket_impl_->Shutdown(), 0);
}

TEST_F(SocketImplCbTestSuite, established) {
	EXPECT_EQ(client_socket_impl_->Established(), 0);
}

TEST_F(SocketImplCbTestSuite, write) {
	EXPECT_EQ(client_socket_impl_->Write(w_content_, (i32)std::strlen(w_content_)), (i32)std::strlen(w_content_));
}

TEST_F(SocketImplCbTestSuite, write2) {
	client_socket_impl_->SetUvData(nullptr);
	EXPECT_EQ(client_socket_impl_->Write(w_content_, (i32)std::strlen(w_content_)), (i32)std::strlen(w_content_));
}

TEST_F(SocketImplCbTestSuite, buff) {
	client_socket_impl_->SetSendBufferSize(8192);
	client_socket_impl_->SetRecvBufferSize(8192);
	i32 send_buff = client_socket_impl_->GetSendBufferSize();
	i32 recv_buff = client_socket_impl_->GetRecvBufferSize();
	EXPECT_GE(send_buff, 8192);
	EXPECT_GE(recv_buff, 8192);
	std::printf("send: %d, recv: %d\n", send_buff, recv_buff);
}

TEST_F(SocketImplCbTestSuite, address) {
	EXPECT_NE(client_socket_impl_->LocalAddress(), Net::SocketAddress());
	EXPECT_NE(client_socket_impl_->RemoteAddress(), Net::SocketAddress());
}

TEST_F(SocketImplCbTestSuite, opt) {
	client_socket_impl_->SetNoDelay();
	client_socket_impl_->SetKeepAlive(60);
}

class SocketImplEstablishedTestSuite : public SocketImplCbTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketImplCbTestSuite::SetUp();
		EXPECT_EQ(client_socket_impl_->Established(), 0);
		EXPECT_EQ(server_data_->client_impl_->Write(w_content_, (i32)std::strlen(w_content_)), (i32)std::strlen(w_content_));
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		SocketImplCbTestSuite::TearDown();
	}
};

TEST_F(SocketImplEstablishedTestSuite, read) {
	Loop();
	EXPECT_GE(client_data_->call_alloc_count_, 1);
	EXPECT_GE(client_data_->call_read_count_, 1);
}

TEST_F(SocketImplEstablishedTestSuite, read2) {
	MockUvData * data = new MockUvData();
	client_socket_impl_->SetUvData(data);
	delete data;
	Loop();
	EXPECT_EQ(client_data_->call_alloc_count_, 0);
	EXPECT_EQ(client_data_->call_read_count_, 0);
}