#ifdef USE_VLD
#include "vld.h"
#endif
#include "Sockets/StreamSocket.h"
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
static i8 * kMsgBuffer = nullptr;			// 数据包
static i32 kClientCount = 0;				// 计划连接数
static i32 kPacketSize = 0;					// 计划数据包大小
static i32 kBufferSize = 65536;				// 缓冲区大小
static bool kLogDetail = false;				// 是否打印日志
static bool kQuit = false;					// 事件循环退出
static std::set<ClientUvData *> kClients;	// 所有套接字
static i32 kIndex = 1;						// 套接字索引

class ClientUvData : public Net::UvData {
public:
	ClientUvData(i32 packet_count) : packet_count_(packet_count), buffer_(nullptr), used_(0), index_(kIndex++) {
		socket_.Open(uv_default_loop());
		socket_.SetUvData(this);
		buffer_ = static_cast<i8 *>(jc_malloc(kBufferSize));
	}
	virtual ~ClientUvData() {
		jc_free(buffer_);
	}
	void CheckQuit() {
		if (kDisconnectedCount + kConnectFailedCount >= kClientCount) {
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
	void ProcessCommand() const {
		if (kLogDetail) {
			PackHeader ph = {0};
			std::memcpy(&ph, buffer_, PACK_HEADER_LEN);
			const int data_len = ph.data_len;
			std::printf("Package Length %d %d\n", index_, data_len);
		}
	}
	virtual void CloseCallback() override {
		++kDisconnectedCount;
		CheckQuit();
		socket_.SetUvData(nullptr);
		kClients.erase(this);
		delete this;
	}
	virtual void ConnectCallback(i32 status, void * arg) override {
		if (status == 0) {
			if (socket_.Established() == 0) {
				++kConnectedCount;
				if (socket_.Write(kMsgBuffer, kPacketSize) > 0) {
					++kWriteCount;
				} else {
					socket_.Shutdown();
				}
			} else {
				++kConnectFailedCount;
				CheckQuit();
				socket_.Close();
			}
		} else {
			++kConnectFailedCount;
			CheckQuit();
			socket_.Close();
		}
	}
	virtual void ShutdownCallback(i32 status, void * arg) override {
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
						ProcessCommand();
						used_ = 0;
					} else if (used_ > message_size) {
						ProcessCommand();
						std::memmove(buffer_, buffer_ + message_size, used_ - message_size);
						used_ -= message_size;
						done = false;
					}
				}
			} while (!done);
		} else if (status < 0) {
			socket_.Close();
		}
	}
	virtual void WrittenCallback(i32 status, void * arg) override {
		if (status == 0) {
			if (--packet_count_ > 0 && socket_.Write(kMsgBuffer, kPacketSize) > 0) {
				++kWriteCount;
			} else {
				socket_.Shutdown();
			}
		} else {
			socket_.Close();
		}
	}

public:
	Net::StreamSocket socket_;
	i32 packet_count_;
	i8 * buffer_;
	i32 used_;
	i32 index_;
};

bool get_parameters(int argc, const char * * argv, std::string & host, i32 & port, i32 & client_count, i32 & packet_count, i32 & packet_size, bool & log_detail) {
	client_count = 1;
	packet_count = 1;
	packet_size = 64;
	log_detail = false;
	for (int i = 0; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (argc < 3 || arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			std::printf("Usage: BenchSocketClient host port [-c client_count] [-p packet_count] [-s packet_size] [--log]\n");
			std::printf("-c client_count: 客户端并发数量，默认 1\n");
			std::printf("-p packet_count: 每个客户端发送数据包数量，默认 1\n");
			std::printf("-s packet_size : 每个数据包大小，默认 64字节\n");
			std::printf("--log          : 是否输出更多日志，默认 否\n");
			return false;
		} else if (arg_string == "-c") {
			client_count = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "-p") {
			packet_count = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "-s") {
			packet_size = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "--log") {
			log_detail = true;
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
	i32 packet_count;
	if (!get_parameters(argc, argv, host, port, kClientCount, packet_count, kPacketSize, kLogDetail)) {
		return 1;
	}
	// 初始化
	kMsgBuffer = static_cast<i8 *>(jc_malloc(kPacketSize));
	PackHeader ph = {0};
	ph.pack_begin_flag = PACK_BEGIN_FLAG;
	ph.pack_end_flag = PACK_END_FLAG;
	ph.data_len = static_cast<u16>(kPacketSize - PACK_HEADER_LEN);
	ph.crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
	std::memset(kMsgBuffer, '.', kPacketSize);
	std::memcpy(kMsgBuffer, &ph, PACK_HEADER_LEN);
	std::memcpy(kMsgBuffer + PACK_HEADER_LEN, "BEGIN", strlen("BEGIN"));
	std::memcpy(kMsgBuffer + kPacketSize - strlen("END"), "END", strlen("END"));
	for (i32 i = 0; i < kClientCount; ++i) {
		ClientUvData * client = new ClientUvData(packet_count);
		if (client->socket_.Connect(Net::SocketAddress(host, port)) == 0) {
			kClients.insert(client);
		} else {
			++kConnectFailedCount;
		}
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
	for (auto & it : kClients) {
		it->socket_.Close();
	}
	kClients.clear();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	jc_free(kMsgBuffer);
	// 状态打印
	std::printf("计划连接数/成功连接数/失败连接数/关闭连接数 %d/%d/%d/%d\n", kClientCount, kConnectedCount, kConnectFailedCount, kDisconnectedCount);
	std::printf("计划发包数/成功发包数 %d/%d\n", packet_count * kClientCount, kWriteCount);
	std::printf("计划发包大小/成功发包大小/成功收包大小 %d/%d/%d\n", packet_count * kPacketSize * kClientCount, kPacketSize * kWriteCount, kReadPacketSize);
	return 0;
}