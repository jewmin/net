#ifdef USE_VLD
#include "vld.h"
#endif
#include <thread>
#include "Common/Allocator.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketConnector.h"
#include "Reactor/SocketConnection.h"

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
static i32 kConnectedCount = 0;					// 成功连接数
static i32 kConnectFailedCount = 0;				// 失败连接数
static i32 kDisconnectedCount = 0;				// 关闭连接数
static i32 kWriteCount = 0;						// 成功发包数
static i64 kReadPacketSize = 0;					// 收包字节数
static i8 * kMsgBuffer = nullptr;				// 数据包
static i32 kClientCount = 0;					// 计划连接数
static i32 kPacketSize = 0;						// 计划数据包大小
static i32 kBufferSize = 65536;					// 缓冲区大小
static bool kLogDetail = false;					// 是否打印日志
static bool kQuit = false;						// 事件循环退出
static std::set<ClientUvData *> kClients;		// 所有套接字
static i32 kIndex = 1;							// 套接字索引
static std::set<ClientUvData *> kDeleteClients;	// 删除的套按字

enum ConnectState { kConnecting, kConnected, kDisconnecting, kDisconnected };

class ClientUvData : public Net::SocketConnection {
public:
	ClientUvData(i32 packet_count) : Net::SocketConnection(kBufferSize, kBufferSize), packet_count_(packet_count), index_(kIndex++) {
	}
	virtual ~ClientUvData() {
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
		if (GetRecvDataSize() > static_cast<i32>(PACK_HEADER_LEN)) {
			PackHeader ph = {0};
			std::memcpy(&ph, GetRecvData(), PACK_HEADER_LEN);
			if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
				u16 crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
				if (crc_data == ph.crc_data) {
					return ph.data_len + PACK_HEADER_LEN;
				}
			}
		}
		return 0;
	}
	void ProcessCommand(const i32 message_size) const {
		kReadPacketSize += message_size;
		if (kLogDetail) {
			PackHeader ph = {0};
			std::memcpy(&ph, GetRecvData(), PACK_HEADER_LEN);
			const int data_len = ph.data_len;
			std::printf("Package Length %d %d\n", index_, data_len);
		}
	}
	virtual void OnConnected() override {
		++kConnectedCount;
		i32 status = Write(kMsgBuffer, kPacketSize);
		if (status > 0) {
			++kWriteCount;
		} else {
			std::printf("OnConnected %d %d %s\n", index_, status, uv_strerror(status));
			Shutdown(false);
		}
	}
	virtual void OnConnectFailed(i32 reason) override {
		std::printf("OnConnectFailed %d %d %s\n", index_, reason, uv_strerror(reason));
		++kConnectFailedCount;
		CheckQuit();
		kClients.erase(this);
		kDeleteClients.insert(this);
	}
	virtual void OnDisconnected(bool is_remote) override {
		++kDisconnectedCount;
		CheckQuit();
		kClients.erase(this);
		kDeleteClients.insert(this);
	}
	virtual void OnNewDataReceived() override {
		bool done;
		do {
			done = true;
			if (GetRecvDataSize() >= GetMinimumMessageSize()) {
				const i32 message_size = GetMessageSize();
				if (GetRecvDataSize() == message_size) {
					ProcessCommand(message_size);
					PopRecvData(message_size);
				} else if (GetRecvDataSize() > message_size) {
					ProcessCommand(message_size);
					PopRecvData(message_size);
					done = false;
				}
			}
		} while (!done);
	}
	virtual void OnSomeDataSent() override {
		if (--packet_count_ > 0) {
			i32 status = Write(kMsgBuffer, kPacketSize);
			if (status > 0) {
				++kWriteCount;
			} else {
				std::printf("OnSomeDataSent %d %d %s\n", index_, status, uv_strerror(status));
				Shutdown(false);
			}
		} else {
			Shutdown(false);
		}
	}
	virtual void OnError(i32 reason) override {
	}

public:
	i32 packet_count_;
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
	auto start = std::chrono::system_clock::now();
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
	Net::EventReactor * reactor = new Net::EventReactor();
	Net::SocketConnector * connector = new Net::SocketConnector(reactor);
	for (i32 i = 0; i < kClientCount; ++i) {
		ClientUvData * client = new ClientUvData(packet_count);
		kClients.insert(client);
		connector->Connect(client, Net::SocketAddress(host, port));
	}
	uv_signal_t handle1, handle2;
	uv_signal_init(reactor->GetUvLoop(), &handle1);
	uv_signal_start_oneshot(&handle1, signal_cb, SIGINT);
	uv_signal_init(reactor->GetUvLoop(), &handle2);
	uv_signal_start_oneshot(&handle2, signal_cb, SIGTERM);
	// 事件循环
	while (!kQuit) {
		reactor->Poll();
		for (auto & it : kDeleteClients) {
			delete it;
		}
		kDeleteClients.clear();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	// 结束
	uv_close(reinterpret_cast<uv_handle_t *>(&handle1), nullptr);
	uv_close(reinterpret_cast<uv_handle_t *>(&handle2), nullptr);
	for (auto & it : kClients) {
		it->Shutdown(true);
	}
	delete connector;
	delete reactor;
	jc_free(kMsgBuffer);
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	// 状态打印
	std::printf("计划连接数/成功连接数/失败连接数/关闭连接数 %d/%d/%d/%d\n", kClientCount, kConnectedCount, kConnectFailedCount, kDisconnectedCount);
	std::printf("计划发包数/成功发包数 %d/%d\n", packet_count * kClientCount, kWriteCount);
	std::printf("计划发包大小/成功发包大小/成功收包大小 %lld/%lld/%lld\n", packet_count * static_cast<i64>(kPacketSize) * kClientCount, static_cast<i64>(kPacketSize) * kWriteCount, kReadPacketSize);
#ifdef _WIN32
	std::printf("耗时(微秒) %lld\n", duration.count());
#else
	std::printf("耗时(微秒) %ld\n", duration.count());
#endif
	return 0;
}