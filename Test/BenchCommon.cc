#include "BenchCommon.h"
#include "Common/Logger.h"
#include "Common/Allocator.h"
#include <chrono>

BenchCommon::BenchCommon(bool log_detail)
	: reactor_(new Net::EventReactor()), quit_(false), log_detail_(log_detail)
	, recv_packet_size_(0), write_counter_(0)
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
	std::printf("成功连接数/失败连接数/关闭连接数 %d/%d/%d\n", connected_counter_, connect_failed_counter_, disconnected_counter_);
	std::printf("成功收包大小 %lld\n", recv_packet_size_);
	std::printf("回包数 %d\n", write_counter_);
	std::printf("耗时(微秒) %lld\n", use_time_);
}

void BenchCommon::OnConnected(Net::Connection * connection) {
}

void BenchCommon::OnConnectFailed(Net::Connection * connection, i32 reason) {
}

void BenchCommon::OnDisconnected(Net::Connection * connection, bool is_remote) {
}

void BenchCommon::OnNewDataReceived(Net::Connection * connection) {
	bool done;
	do {
		done = true;
		const i32 size = connection->GetRecvDataSize();
		if (size >= GetMinimumMessageSize()) {
			const i32 message_size = GetMessageSize(connection);
			if (size == message_size) {
				ProcessCommand(connection, message_size);
				connection->PopRecvData(message_size);
			} else if (size > message_size) {
				ProcessCommand(connection, message_size);
				connection->PopRecvData(message_size);
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
	if (connection->GetRecvDataSize() > static_cast<i32>(PACK_HEADER_LEN)) {
		PackHeader ph = {0};
		std::memcpy(&ph, connection->GetRecvData(), PACK_HEADER_LEN);
		if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
			u16 crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
			if (crc_data == ph.crc_data) {
				return ph.data_len + PACK_HEADER_LEN;
			}
		}
	}
	return 0;
}

void BenchCommon::ProcessCommand(Net::Connection * connection, const i32 message_size) {
	recv_packet_size_ += message_size;
	if (log_detail_) {
		PackHeader ph = {0};
		std::memcpy(&ph, connection->GetRecvData(), PACK_HEADER_LEN);
		const int data_len = ph.data_len;
		std::printf("Package Length %lld %d\n", connection->GetConnectionId(), data_len);
	}
}

void BenchCommon::Send(Net::Connection * connection, const i8 * data, i32 len) {
	i32 status = connection->Write(data, len);
	if (status > 0) {
		++write_counter_;
	} else {
		std::printf("Send %lld %d %s\n", connection->GetConnectionId(), status, uv_strerror(status));
		connection->Shutdown(false);
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