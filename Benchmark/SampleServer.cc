#include "SampleServer.h"
#include <chrono>
#include "ProtocolDef.h"
#include "Logger.h"

SampleServer::SampleServer()
	: reactor_(new Net::EventReactor())
	, server_(new Net::SocketServer("SampleServer", reactor_, 65536, 65536))
	, use_time_(0), recv_packet_size_(0), quit_(false)
	, connected_counter_(0), connect_failed_counter_(0), disconnected_counter_(0), packet_counter_(0) {
	server_->SetEvent(this);
}

SampleServer::~SampleServer() {
	delete server_;
	delete reactor_;
}

void SampleServer::Run(const std::string & address, int port) {
	uv_signal_t * sig_handler = new uv_signal_t();
	uv_signal_init(reactor_->GetEventLoop(), sig_handler);
	uv_signal_start_oneshot(sig_handler, SignalCb, SIGINT);
	uv_handle_set_data(reinterpret_cast<uv_handle_t *>(sig_handler), this);
	server_->Listen(address, port);
	auto start = std::chrono::system_clock::now();
	while (!quit_) {
		reactor_->Dispatch();
	}
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	use_time_ = duration.count();
	uv_close(reinterpret_cast<uv_handle_t *>(sig_handler), CloseCb);
}

void SampleServer::ShowStatus() {
	Foundation::LogInfo("成功连接数：%d", connected_counter_);
	Foundation::LogInfo("失败连接数：%d", connect_failed_counter_);
	Foundation::LogInfo("关闭连接数：%d", disconnected_counter_);
	Foundation::LogInfo("耗时(微秒)：%lld", use_time_);
	Foundation::LogInfo("收到数据包总大小(字节)：%lld", recv_packet_size_);
	Foundation::LogInfo("收到数据包：%d", packet_counter_);
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
	Foundation::LogInfo("网卡流量：%.0lf%c, QPS：%.lf", bits, unit, qps);
}

void SampleServer::OnConnected(Net::SocketWrapper * wrapper) {
	++connected_counter_;
}

void SampleServer::OnConnectFailed(Net::SocketWrapper * wrapper, int reason) {
	++connect_failed_counter_;
}

void SampleServer::OnDisconnected(Net::SocketWrapper * wrapper, bool isRemote) {
	++disconnected_counter_;
	if (disconnected_counter_ == connected_counter_) {
		quit_ = true;
	}
}

void SampleServer::OnNewDataReceived(Net::SocketWrapper * wrapper) {
	const int data_size = wrapper->GetRecvDataSize();
	if (data_size >= PACK_HEADER_LEN) {
		const int message_size = GetMessageSize(wrapper);
		if (0 == message_size) {
			Foundation::LogErr("Socket [%u] Get Message Error And Shutdown Now", wrapper->GetId());
			wrapper->ShutdownNow();
		} else if (data_size >= message_size) {
			ProcessCommand(wrapper);
			wrapper->PopRecvData(message_size);
			recv_packet_size_ += message_size;
			++packet_counter_;
		}
	}
}

void SampleServer::OnSomeDataSent(Net::SocketWrapper * wrapper) {
}

int SampleServer::GetMessageSize(Net::SocketWrapper * wrapper) const {
	PackHeader ph = {0};
	memcpy(&ph, wrapper->GetRecvData(), PACK_HEADER_LEN);
	if (PACK_BEGIN_FLAG == ph.pack_begin_flag && PACK_END_FLAG == ph.pack_end_flag) {
		u16 crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
		if (crc_data == ph.crc_data) {
			return ph.data_len + PACK_HEADER_LEN;
		}
	}
	return 0;
}

void SampleServer::ProcessCommand(Net::SocketWrapper * wrapper) const {
	PackHeader ph = {0};
	memcpy(&ph, wrapper->GetRecvData(), PACK_HEADER_LEN);
	const char * data = wrapper->GetRecvData() + PACK_HEADER_LEN;
	const int data_len = ph.data_len;
	/*Foundation::LogInfo("Socket [%u] Package [length:%d]", wrapper->GetId(), data_len);*/
}

void SampleServer::SignalCb(uv_signal_t * handle, int signum) {
	SampleServer * ss = static_cast<SampleServer *>(handle->data);
	if (ss) {
		ss->quit_ = true;
	}
}

void SampleServer::CloseCb(uv_handle_t * handle) {
	delete handle;
}