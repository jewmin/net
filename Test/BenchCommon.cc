#include "BenchCommon.h"
#include "Common/Logger.h"
#include "Common/Allocator.h"
#include <chrono>

BenchCommon::BenchCommon(bool log_detail)
	: reactor_(new Net::EventReactor()), quit_(false), log_detail_(log_detail)
	, recv_packet_size_(0), recv_packet_count_(0)
	, connected_counter_(0), connect_failed_counter_(0), disconnected_counter_(0)
	, sig_int_(nullptr), sig_term_(nullptr), use_time_(0) {
}

BenchCommon::~BenchCommon() {
	DeleteSignal(sig_int_);
	DeleteSignal(sig_term_);
	delete reactor_;
}

void BenchCommon::Poll() {
	sig_int_ = CreateSignal(SIGINT);
	sig_term_ = CreateSignal(SIGTERM);
	auto start = std::chrono::system_clock::now();
	while (!quit_) {
		reactor_->Poll();
	}
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	use_time_ = duration.count();
}

void BenchCommon::ShowStatus() {
	Net::Log(Net::kLog, __FILE__, __LINE__, "成功连接数:", connected_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "失败连接数:", connect_failed_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "关闭连接数:", disconnected_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "耗时(微秒):", use_time_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "发送/收到数据包:", recv_packet_count_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "发送/收到数据包总大小(字节):", recv_packet_size_);
	if (use_time_ > 0) {
		double bytes = (double)recv_packet_size_ * 1000000 / (double)use_time_;
		double mbytes = bytes / 1048576;
		double qps = (double)recv_packet_count_ * 1000000 / (double)use_time_;
		std::printf("Network traffic: %.2lf(%.2lfM), QPS: %.lf\n", bytes, mbytes, qps);
	}
}

void BenchCommon::OnConnected(Net::Connection * connection) {
	++connected_counter_;
}

void BenchCommon::OnConnectFailed(Net::Connection * connection, i32 reason) {
	++connect_failed_counter_;
}

void BenchCommon::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
	if (disconnected_counter_ == connected_counter_) {
		quit_ = true;
	}
}

void BenchCommon::OnNewDataReceived(Net::Connection * connection) {
	bool done;
	do {
		done = true;
		const i32 size = connection->GetRecvDataSize();
		if (size >= GetMinimumMessageSize()) {
			const i32 message_size = GetMessageSize(connection);
			if (size == message_size) {
				ProcessCommand(connection);
				connection->PopRecvData(message_size);
				recv_packet_size_ += message_size;
				++recv_packet_count_;
			} else if (size > message_size) {
				ProcessCommand(connection);
				connection->PopRecvData(message_size);
				recv_packet_size_ += message_size;
				++recv_packet_count_;
				done = false;
			}
		}
	} while (!done);
}

void BenchCommon::OnSomeDataSent(Net::Connection * connection) {
}

i32 BenchCommon::GetMinimumMessageSize() const {
	return static_cast<i32>(PACK_HEADER_LEN);
}

i32 BenchCommon::GetMessageSize(Net::Connection * connection) const {
	const i8 * data = connection->GetRecvData();
	const i32 size = connection->GetRecvDataSize();
	if (size > static_cast<i32>(PACK_HEADER_LEN)) {
		PackHeader ph = {0};
		std::memcpy(&ph, data, PACK_HEADER_LEN);
		if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
			u16 crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
			if (crc_data == ph.crc_data) {
				return ph.data_len + PACK_HEADER_LEN;
			}
		}
	}
	return 0;
}

void BenchCommon::ProcessCommand(Net::Connection * connection) const {
	if (log_detail_) {
		PackHeader ph = {0};
		std::memcpy(&ph, connection->GetRecvData(), PACK_HEADER_LEN);
		const int data_len = ph.data_len;
		Net::Log(Net::kLog, __FILE__, __LINE__, "Socket [", connection->GetConnectionId(), "] Package Length", data_len);
	}
}

uv_signal_t * BenchCommon::CreateSignal(i32 signum) {
	uv_signal_t * handle = static_cast<uv_signal_t *>(jc_malloc(sizeof(uv_signal_t)));
	uv_signal_init(reactor_->GetUvLoop(), handle);
	uv_signal_start_oneshot(handle, SignalCb, signum);
	uv_handle_set_data(reinterpret_cast<uv_handle_t *>(handle), this);
	return handle;
}

void BenchCommon::DeleteSignal(uv_signal_t * handle) {
	if (handle) {
		if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(handle))) {
			uv_close(reinterpret_cast<uv_handle_t *>(handle), CloseCb);
		}
	}
}

void BenchCommon::SignalCb(uv_signal_t * handle, int signum) {
	BenchCommon * data = static_cast<BenchCommon *>(handle->data);
	if (data) {
		data->quit_ = true;
	}
}

void BenchCommon::CloseCb(uv_handle_t * handle) {
	jc_free(handle);
}