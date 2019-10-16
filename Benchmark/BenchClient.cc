#include "BenchClient.h"
#include "ProtocolDef.h"
#include "Core/SocketClient.h"
#include "Logger.h"

BenchClient::BenchClient(int client_count, int packet_count, int packet_size)
	: reactor_(new Net::EventReactor()), connector_(new Net::SocketConnector(reactor_)), quit_(false)
	, client_count_(client_count), packet_count_(packet_count), packet_size_(packet_size)
	, connected_counter_(0), connect_failed_counter_(0), disconnected_counter_(0) {
	if (packet_size_ <= PACK_HEADER_LEN) {
		packet_size_ = PACK_HEADER_LEN + 10;
	} else if (packet_size_ >= 65536) {
		packet_size_ = 65535;
	}
	buffer_ = static_cast<char *>(malloc(packet_size_));
	PackHeader ph = {0};
	ph.pack_begin_flag = PACK_BEGIN_FLAG;
	ph.pack_end_flag = PACK_END_FLAG;
	ph.data_len = packet_size_ - PACK_HEADER_LEN;
	ph.crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
	memset(buffer_, '.', packet_size_);
	memcpy(buffer_, &ph, PACK_HEADER_LEN);
	memcpy(buffer_ + PACK_HEADER_LEN, "BEGIN", strlen("BEGIN"));
	memcpy(buffer_ + packet_size_ - strlen("END"), "END", strlen("END"));
}

BenchClient::~BenchClient() {
	connector_->Destroy();
	delete reactor_;
	delete buffer_;
}

void BenchClient::Run(const std::string & address, int port) {
	u32 id;
	std::vector<Net::SocketClient *> clients;
	clients.reserve(client_count_);
	for (int i = 0; i < client_count_; ++i) {
		Net::SocketClient * client = new Net::SocketClient("BenchClient", reactor_, connector_, 65536, 65536);
		client->SetEvent(this);
		client->Connect(address, port, id);
		clients.push_back(client);
	}
	while (!quit_) {
		reactor_->Dispatch();
	}
	for (auto & it: clients) {
		delete it;
	}
}

void BenchClient::ShowStatus() {
	Foundation::LogInfo("成功连接数：%d", connected_counter_);
	Foundation::LogInfo("失败连接数：%d", connect_failed_counter_);
	Foundation::LogInfo("关闭连接数：%d", disconnected_counter_);
	Foundation::LogInfo("要连接数：%d", client_count_);
	Foundation::LogInfo("要发包数：%d", packet_count_);
	Foundation::LogInfo("要发的数据包大小：%d", packet_size_);
	/*for (auto & it : client_packet_map_) {
		Foundation::LogInfo("已发包数：%d", it.second);
	}*/
}

void BenchClient::OnConnected(Net::SocketWrapper * wrapper) {
	/*++connected_counter_;
	if (connected_counter_ + connect_failed_counter_ == client_count_) {
		quit_ = true;
	}*/
	client_packet_map_.insert(std::pair<u64, int>(reinterpret_cast<u64>(wrapper), 0));
	wrapper->Write(buffer_, packet_size_);
}

void BenchClient::OnConnectFailed(Net::SocketWrapper * wrapper, int reason) {
	++connect_failed_counter_;
	if (connected_counter_ + connect_failed_counter_ == client_count_) {
		quit_ = true;
	}
}

void BenchClient::OnDisconnected(Net::SocketWrapper * wrapper, bool isRemote) {
	++disconnected_counter_;
}

void BenchClient::OnNewDataReceived(Net::SocketWrapper * wrapper) {
	const int data_size = wrapper->GetRecvDataSize();
	if (data_size >= PACK_HEADER_LEN) {
		const int message_size = GetMessageSize(wrapper);
		if (0 == message_size) {
			Foundation::LogErr("Socket [%u] Get Message Error And Shutdown Now", wrapper->GetId());
			wrapper->ShutdownNow();
		} else if (data_size >= message_size) {
			ProcessCommand(wrapper);
			wrapper->PopRecvData(message_size);
		}
	}
}

void BenchClient::OnSomeDataSent(Net::SocketWrapper * wrapper) {
	auto it = client_packet_map_.find(reinterpret_cast<u64>(wrapper));
	if (it != client_packet_map_.end()) {
		if (++it->second < packet_count_) {
			wrapper->Write(buffer_, packet_size_);
		} else {
			++connected_counter_;
			if (connected_counter_ + connect_failed_counter_ == client_count_) {
				quit_ = true;
			}
		}
	}
}

int BenchClient::GetMessageSize(Net::SocketWrapper * wrapper) const {
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

void BenchClient::ProcessCommand(Net::SocketWrapper * wrapper) const {
	PackHeader ph = {0};
	memcpy(&ph, wrapper->GetRecvData(), PACK_HEADER_LEN);
	const char * data = wrapper->GetRecvData() + PACK_HEADER_LEN;
	const int data_len = ph.data_len;
	/*Foundation::LogInfo("Socket [%u] Package [length:%d]", wrapper->GetId(), data_len);*/
}