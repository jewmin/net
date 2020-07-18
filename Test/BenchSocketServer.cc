#ifdef USE_VLD
#include "vld.h"
#endif
#include "Sockets/StreamSocket.h"
#include "Sockets/ServerSocket.h"
#include "Common/Allocator.h"
#include <thread>

#pragma pack(1)

typedef struct tagPackHeader {
	u8 pack_begin_flag;	// 0xBF
	u16 data_len;		// [PackHeader] [DATA] data_len == sizeof(DATA)
	u16 crc_data;		// [pack_begin_flag | pack_end_flag] | [data_len]
	u8 pack_end_flag;	// 0xEF
} PackHeader;

#define PACK_BEGIN_FLAG					0xBF
#define PACK_END_FLAG					0xEF
#define PRIVATE_MAKE_CRC_DATA(x, y, z)	(((x) << 8 | (y)) | (z))
#define MAKE_CRC_DATA(x, y, z)			PRIVATE_MAKE_CRC_DATA(x, y, z)
#define PACK_HEADER_LEN					sizeof(PackHeader)

#pragma pack()

class ClientUvData;
static i32 kConnectedCount = 0;				// 成功连接数
static i32 kConnectFailedCount = 0;			// 失败连接数
static i32 kDisconnectedCount = 0;			// 关闭连接数
static i32 kWriteCount = 0;					// 成功发包数
static i32 kReadPacketSize = 0;				// 收包字节数
static i32 kBufferSize = 65536;				// 缓冲区大小
static bool kLogDetail = false;				// 是否打印日志
static bool kAutoClose = false;				// 是否自动退出
static bool kQuit = false;					// 事件循环退出
static std::set<ClientUvData *> kClients;	// 所有套接字
static i32 kIndex = 1;						// 套接字索引

enum ConnectState { kConnecting, kConnected, kDisconnecting, kDisconnected };

class ClientUvData : public Net::UvData {
public:
	ClientUvData(Net::StreamSocket socket) : socket_(socket), buffer_(nullptr), used_(0), index_(kIndex++), status_(kDisconnected) {
		socket_.SetUvData(this);
		buffer_ = static_cast<i8 *>(jc_malloc(kBufferSize));
	}
	virtual ~ClientUvData() {
		jc_free(buffer_);
	}
	void CheckQuit() {
		if (kDisconnectedCount == kConnectedCount && kAutoClose) {
			kQuit = true;
		}
	}
	i32 GetMinimumMessageSize() const {
		return static_cast<i32>(PACK_HEADER_LEN);
	}
	i32 GetMessageSize() const {
		if (used_ > static_cast<i32>(PACK_HEADER_LEN)) {
			PackHeader ph = {0};
			std::memcpy(&ph, buffer_, PACK_HEADER_LEN);
			if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
				u16 crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
				if (crc_data == ph.crc_data) {
					return ph.data_len + PACK_HEADER_LEN;
				}
			}
		}
		return 0;
	}
	void ProcessCommand(const i32 message_size) {
		if (kLogDetail) {
			PackHeader ph = {0};
			std::memcpy(&ph, buffer_, PACK_HEADER_LEN);
			const int data_len = ph.data_len;
			std::printf("Package Length %d %d\n", index_, data_len);
		}
		Send(message_size);
	}
	void Send(const i32 message_size) {
		if (status_ == kConnected) {
			i8 * msg = static_cast<i8 *>(jc_malloc(message_size));
			std::memcpy(msg, buffer_, message_size);
			i32 status = 0;
			if ((status = socket_.Write(msg, message_size, msg)) > 0) {
				++kWriteCount;
			} else {
				jc_free(msg);
				std::printf("Send %d %d %s\n", index_, status, uv_strerror(status));
				status_ = kDisconnecting;
				socket_.Shutdown();
			}
		}
	}
	virtual void CloseCallback() override {
		++kDisconnectedCount;
		CheckQuit();
		socket_.SetUvData(nullptr);
		kClients.erase(this);
		delete this;
	}
	virtual void ShutdownCallback(i32 status, void * arg) override {
		status_ = kDisconnected;
		socket_.Close();
	}
	virtual void AllocCallback(uv_buf_t * buf) override {
		buf->base = buffer_ + used_;
		buf->len = kBufferSize - used_;
	}
	virtual void ReadCallback(i32 status) override {
		if (status > 0) {
			used_ += status;
			kReadPacketSize += status;
			bool done;
			do {
				done = true;
				if (used_ >= GetMinimumMessageSize()) {
					const i32 message_size = GetMessageSize();
					if (used_ == message_size) {
						ProcessCommand(message_size);
						used_ = 0;
					} else if (used_ > message_size) {
						ProcessCommand(message_size);
						std::memmove(buffer_, buffer_ + message_size, used_ - message_size);
						used_ -= message_size;
						done = false;
					}
				}
			} while (!done);
		} else if (status < 0) {
			std::printf("ReadCallback %d %d %s\n", index_, status, uv_strerror(status));
			status_ = kDisconnected;
			socket_.Close();
		}
	}
	virtual void WrittenCallback(i32 status, void * arg) override {
		jc_free(arg);
		if (status < 0) {
			std::printf("WrittenCallback %d %d %s\n", index_, status, uv_strerror(status));
			status_ = kDisconnected;
			socket_.Close();
		}
	}

public:
	Net::StreamSocket socket_;
	i8 * buffer_;
	i32 used_;
	i32 index_;
	i32 status_;
};

class ServerUvData : public Net::UvData {
public:
	ServerUvData() {
		socket_.Open(uv_default_loop());
		socket_.SetUvData(this);
	}
	virtual ~ServerUvData() {
	}
	virtual void AcceptCallback(i32 status) override {
		if (status == 0) {
			Net::StreamSocket client;
			if (socket_.AcceptSocket(client)) {
				ClientUvData * connection = new ClientUvData(client);
				if ((status = connection->socket_.Established()) == 0) {
					++kConnectedCount;
					connection->status_ = kConnected;
					kClients.insert(connection);
				} else {
					std::printf("AcceptCallback %d %d %s\n", kIndex, status, uv_strerror(status));
					++kConnectFailedCount;
					connection->socket_.Close();
				}
			} else {
				++kConnectFailedCount;
			}
		} else {
			std::printf("AcceptCallback %d %d %s\n", kIndex, status, uv_strerror(status));
			socket_.Close();
		}
	}
	virtual void CloseCallback() override {
		socket_.SetUvData(nullptr);
		delete this;
	}

public:
	Net::ServerSocket socket_;
};

bool get_parameters(int argc, const char * * argv, std::string & host, i32 & port, bool & log_detail, bool & auto_close) {
	log_detail = false;
	auto_close = false;
	for (int i = 0; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (argc < 3 || arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			std::printf("Usage: BenchServer host port [--log]\n");
			std::printf("--log          : 是否输出更多日志，默认 否\n");
			std::printf("--auto         : 是否自动退出，默认 否\n");
			return false;
		} else if (arg_string == "--log") {
			log_detail = true;
			remove_flag = true;
		}  else if (arg_string == "--auto") {
			auto_close = true;
			remove_flag = true;
		} else if (i == 1) {
			host = arg_string;
		} else if (i == 2) {
			port = std::atoi(argv[i]);
		}
		if (remove_flag) {
			for (int j = i; j != argc; j++) {
				argv[j] = argv[j + 1];
			}
			argc--;
			i--;
		}
	}
	return true;
}

void signal_cb(uv_signal_t * handle, int signum) {
	kQuit = true;
}

int main(int argc, const char * * argv) {
	std::string host;
	i32 port;
	if (!get_parameters(argc, argv, host, port, kLogDetail, kAutoClose)) {
		return 1;
	}
	// 初始化
	ServerUvData * server = new ServerUvData();
	if (server->socket_.Bind(Net::SocketAddress(host, port)) < 0) {
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		return 1;
	}
	if (server->socket_.Listen(512) < 0) {
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		return 1;
	}
	uv_signal_t handle1, handle2;
	uv_signal_init(uv_default_loop(), &handle1);
	uv_signal_start_oneshot(&handle1, signal_cb, SIGINT);
	uv_signal_init(uv_default_loop(), &handle2);
	uv_signal_start_oneshot(&handle2, signal_cb, SIGTERM);
	// 事件循环
	while (!kQuit) {
		uv_run(uv_default_loop(), UV_RUN_NOWAIT);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	// 结束
	uv_close(reinterpret_cast<uv_handle_t *>(&handle1), nullptr);
	uv_close(reinterpret_cast<uv_handle_t *>(&handle2), nullptr);
	server->socket_.Close();
	for (auto & it : kClients) {
		it->socket_.Close();
	}
	kClients.clear();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
	// 状态打印
	std::printf("成功连接数/失败连接数/关闭连接数 %d/%d/%d\n", kConnectedCount, kConnectFailedCount, kDisconnectedCount);
	std::printf("成功收包大小 %d\n", kReadPacketSize);
	return 0;
}