#ifdef USE_VLD
#include "vld.h"
#endif
#include <thread>
#include "Common/Allocator.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketAcceptor.h"
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
static i32 kBufferSize = 65536;					// 缓冲区大小
static bool kLogDetail = false;					// 是否打印日志
static bool kAutoClose = false;					// 是否自动退出
static bool kEcho = false;						// 是否回包
static bool kQuit = false;						// 事件循环退出
static std::set<ClientUvData *> kClients;		// 所有套接字
static i32 kIndex = 1;							// 套接字索引
static std::set<ClientUvData *> kDeleteClients;	// 删除的套按字

enum ConnectState { kConnecting, kConnected, kDisconnecting, kDisconnected };

class ClientUvData : public Net::SocketConnection {
public:
	ClientUvData() : Net::SocketConnection(65536, 65536), index_(kIndex++) {
	}
	virtual ~ClientUvData() {
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
	void ProcessCommand(const i32 message_size) {
		kReadPacketSize += message_size;
		if (kLogDetail) {
			PackHeader ph = {0};
			std::memcpy(&ph, GetRecvData(), PACK_HEADER_LEN);
			const int data_len = ph.data_len;
			std::printf("Package Length %d %d\n", index_, data_len);
		}
		if (kEcho) {
			Send(message_size);
		}
	}
	void Send(const i32 message_size) {
		i32 status = Write(GetRecvData(), message_size);
		if (status > 0) {
			++kWriteCount;
		} else {
			std::printf("Send %d %d %s\n", index_, status, uv_strerror(status));
			Shutdown(false);
		}
	}
	virtual void OnConnected() override {
		++kConnectedCount;
		CheckQuit();
		kClients.insert(this);
	}
	virtual void OnConnectFailed(i32 reason) override {
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
	}
	virtual void OnError(i32 reason) override {
	}

public:
	i32 index_;
};

class ServerUvData : public Net::SocketAcceptor {
public:
	ServerUvData(Net::EventReactor * reactor) : Net::SocketAcceptor(reactor) {
	}
	virtual ~ServerUvData() {
	}
	virtual Net::SocketConnection * CreateConnection() override {
		return new ClientUvData();
	}
	virtual void DestroyConnection(Net::SocketConnection * connection) override {
		delete this;
	}
};

bool get_parameters(int argc, const char * * argv, std::string & host, i32 & port, bool & log_detail, bool & auto_close, bool & echo) {
	log_detail = false;
	auto_close = false;
	echo = false;
	for (int i = 0; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (argc < 3 || arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			std::printf("Usage: BenchServer host port [--log]\n");
			std::printf("--log          : 是否输出更多日志，默认 否\n");
			std::printf("--auto         : 是否自动退出，默认 否\n");
			std::printf("--echo         : 是否回包，默认 否\n");
			return false;
		} else if (arg_string == "--log") {
			log_detail = true;
			remove_flag = true;
		} else if (arg_string == "--auto") {
			auto_close = true;
			remove_flag = true;
		} else if (arg_string == "--echo") {
			echo = true;
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
	if (!get_parameters(argc, argv, host, port, kLogDetail, kAutoClose, kEcho)) {
		return 1;
	}
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
	auto start = std::chrono::system_clock::now();
	// 初始化
	Net::EventReactor * reactor = new Net::EventReactor();
	ServerUvData * server = new ServerUvData(reactor);
	if (!server->Open(Net::SocketAddress(host, port), 512)) {
		delete server;
		delete reactor;
		return 1;
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
	delete server;
	delete reactor;
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	// 状态打印
	std::printf("成功连接数/失败连接数/关闭连接数 %d/%d/%d\n", kConnectedCount, kConnectFailedCount, kDisconnectedCount);
	std::printf("成功收包大小 %lld\n", kReadPacketSize);
	std::printf("回包数 %d\n", kWriteCount);
#ifdef _WIN32
	std::printf("耗时(微秒) %lld\n", duration.count());
#else
	std::printf("耗时(微秒) %ld\n", duration.count());
#endif
	return 0;
}