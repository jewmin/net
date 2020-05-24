#include "BenchServer.h"
#include <chrono>
#include "ProtocolDef.h"
#include "Common/Logger.h"
#include "Common/Allocator.h"

BenchServer::BenchServer(bool logDetail)
	: reactor_(new Net::EventReactor())
	, server_(new Net::Server("BenchServer", reactor_, 65536, 65536))
	, use_time_(0), recv_packet_size_(0), quit_(false), log_detail_(logDetail)
	, connected_counter_(0), connect_failed_counter_(0), disconnected_counter_(0), packet_counter_(0) {
	server_->SetNotification(this);
}

BenchServer::~BenchServer() {
	delete server_;
	delete reactor_;
}

void BenchServer::Run(const std::string & address, i32 port) {
	uv_signal_t * sig_handler = static_cast<uv_signal_t *>(jc_malloc(sizeof(uv_signal_t)));
	uv_signal_init(reactor_->GetUvLoop(), sig_handler);
	uv_signal_start_oneshot(sig_handler, SignalCb, SIGINT);
	uv_handle_set_data(reinterpret_cast<uv_handle_t *>(sig_handler), this);

	uv_signal_t * sig_handler2 = static_cast<uv_signal_t *>(jc_malloc(sizeof(uv_signal_t)));
	uv_signal_init(reactor_->GetUvLoop(), sig_handler2);
	uv_signal_start_oneshot(sig_handler2, SignalCb, SIGTERM);
	uv_handle_set_data(reinterpret_cast<uv_handle_t *>(sig_handler2), this);


	server_->Listen(address, port);
	auto start = std::chrono::system_clock::now();
	while (!quit_) {
		reactor_->Poll();
	}
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	use_time_ = duration.count();

	uv_close(reinterpret_cast<uv_handle_t *>(sig_handler), CloseCb);
	uv_close(reinterpret_cast<uv_handle_t *>(sig_handler2), CloseCb);
}

void BenchServer::ShowStatus() {
	Net::Log(Net::kLog, __FILE__, __LINE__, "成功连接数:", connected_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "失败连接数:", connect_failed_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "关闭连接数:", disconnected_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "耗时(微秒):", use_time_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "收到数据包:", packet_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "收到数据包总大小(字节):", recv_packet_size_);

	double bits = (double)recv_packet_size_ * 1000000 / (double)use_time_;
	char unit = 0;
	for (int i = 0; i < 3; ++i) {
		if (bits >= 1024) {
			bits /= 1024;
			++unit;
		}
	}
	if (unit == 1) {
		unit = 'K';
	} else if (unit == 2) {
		unit = 'M';
	} else if (unit == 3) {
		unit = 'G';
	} else {
		unit = ' ';
	}
	double qps = (double)packet_counter_ * 1000000 / (double)use_time_;
	std::printf("Network traffic: %.2lf%c, QPS: %.lf\n", bits, unit, qps);
}


void BenchServer::OnConnected(Net::Connection * connection) {
	++connected_counter_;
}

void BenchServer::OnConnectFailed(Net::Connection * connection, i32 reason) {
	++connect_failed_counter_;
}

void BenchServer::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
	if (disconnected_counter_ == connected_counter_) {
		quit_ = true;
	}
}

void BenchServer::OnNewDataReceived(Net::Connection * connection) {
	const i32 data_size = connection->GetRecvDataSize();
	if (data_size >= static_cast<i32>(PACK_HEADER_LEN)) {
		const i32 message_size = GetMessageSize(connection);
		if (0 == message_size) {
			Net::Log(Net::kLog, __FILE__, __LINE__, "Socket [", connection->GetConnectionId(), "] Get Message Error And Shutdown Now");
			connection->Shutdown(true);
		} else if (data_size >= message_size) {
			ProcessCommand(connection);
			connection->PopRecvData(message_size);
			recv_packet_size_ += message_size;
			++packet_counter_;
		}
	}
}

void BenchServer::OnSomeDataSent(Net::Connection * connection) {
}

i32 BenchServer::GetMessageSize(Net::Connection * connection) const {
	PackHeader ph = {0};
	std::memcpy(&ph, connection->GetRecvData(), PACK_HEADER_LEN);
	if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
		u16 crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
		if (crc_data == ph.crc_data) {
			return ph.data_len + PACK_HEADER_LEN;
		}
	}
	return 0;
}

void BenchServer::ProcessCommand(Net::Connection * connection) const {
	if (log_detail_) {
		PackHeader ph = {0};
		std::memcpy(&ph, connection->GetRecvData(), PACK_HEADER_LEN);
		const char * data = connection->GetRecvData() + PACK_HEADER_LEN;
		const int data_len = ph.data_len;
		Net::Log(Net::kLog, __FILE__, __LINE__, "Socket [", connection->GetConnectionId(), "] Package Length", data_len);
	}
}

void BenchServer::SignalCb(uv_signal_t * handle, int signum) {
	BenchServer * server = static_cast<BenchServer *>(handle->data);
	if (server) {
		server->quit_ = true;
	}
}

void BenchServer::CloseCb(uv_handle_t * handle) {
	jc_free(handle);
}