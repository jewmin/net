#include "BenchClient.h"
#include "ProtocolDef.h"
#include "Interface/Client.h"
#include "Common/Logger.h"
#include "Common/Allocator.h"

BenchClient::BenchClient(i32 clientCount, i32 packetCount, i32 packetSize, bool logDetail)
	: reactor_(new Net::EventReactor()), connector_(new Net::SocketConnector(reactor_)), quit_(false)
	, log_detail_(logDetail), client_count_(clientCount), packet_count_(packetCount), packet_size_(packetSize)
	, connected_counter_(0), connect_failed_counter_(0), disconnected_counter_(0) {
	if (packet_size_ <= static_cast<i32>(PACK_HEADER_LEN)) {
		packet_size_ = PACK_HEADER_LEN + 10;
	} else if (packet_size_ >= 65536) {
		packet_size_ = 65535;
	}
	msg_buffer_ = static_cast<i8 *>(jc_malloc(packet_size_));
	PackHeader ph = {0};
	ph.pack_begin_flag = PACK_BEGIN_FLAG;
	ph.pack_end_flag = PACK_END_FLAG;
	ph.data_len = static_cast<u16>(packet_size_ - PACK_HEADER_LEN);
	ph.crc_data = MAKE_CRC_DATA(PACK_BEGIN_FLAG, PACK_END_FLAG, ph.data_len);
	std::memset(msg_buffer_, '.', packet_size_);
	std::memcpy(msg_buffer_, &ph, PACK_HEADER_LEN);
	std::memcpy(msg_buffer_ + PACK_HEADER_LEN, "BEGIN", strlen("BEGIN"));
	std::memcpy(msg_buffer_ + packet_size_ - strlen("END"), "END", strlen("END"));
}

BenchClient::~BenchClient() {
	delete connector_;
	delete reactor_;
	jc_free(msg_buffer_);
}

void BenchClient::Run(const std::string & address, int port) {
	Net::Client * client = nullptr;
	std::vector<Net::Client *> clients;
	clients.reserve(client_count_);
	for (int i = 0; i < client_count_; ++i) {
		std::stringstream ss;
		ss << "BenchClient_" << i;
		client = new Net::Client(ss.str(), reactor_, connector_, 65536, 65536);
		client->SetMgrId(i);
		client->SetNotification(this);
		client->Connect(address, port);
		clients.push_back(client);
	}
	while (!quit_) {
		reactor_->Poll();
	}
	for (auto & it: clients) {
		delete it;
	}
}

void BenchClient::ShowStatus() {
	Net::Log(Net::kLog, __FILE__, __LINE__, "成功连接数:", connected_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "失败连接数:", connect_failed_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "关闭连接数:", disconnected_counter_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "要连接数:", client_count_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "要发包数:", packet_count_);
	Net::Log(Net::kLog, __FILE__, __LINE__, "要发的数据包大小:", packet_size_);
	if (log_detail_) {
		for (auto & it : client_packet_map_) {
			Net::Log(Net::kLog, __FILE__, __LINE__, "已发包数:", it.first, it.second);
		}
	}
}

void BenchClient::OnConnected(Net::Connection * connection) {
	client_packet_map_.insert({connection->GetMgr()->GetMgrId(), 0});
	connection->Write(msg_buffer_, packet_size_);
}

void BenchClient::OnConnectFailed(Net::Connection * connection, i32 reason) {
	++connect_failed_counter_;
	if (connected_counter_ + connect_failed_counter_ == client_count_) {
		quit_ = true;
	}
}

void BenchClient::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
}

void BenchClient::OnNewDataReceived(Net::Connection * connection) {
	const i32 data_size = connection->GetRecvDataSize();
	if (data_size >= static_cast<i32>(PACK_HEADER_LEN)) {
		const i32 message_size = GetMessageSize(connection);
		if (0 == message_size) {
			Net::Log(Net::kLog, __FILE__, __LINE__, "Socket [", connection->GetConnectionId(), "] Get Message Error And Shutdown Now");
			connection->Shutdown(true);
		} else if (data_size >= message_size) {
			ProcessCommand(connection);
			connection->PopRecvData(message_size);
		}
	}
}

void BenchClient::OnSomeDataSent(Net::Connection * connection) {
	auto it = client_packet_map_.find(connection->GetMgr()->GetMgrId());
	if (it != client_packet_map_.end()) {
		if (++it->second < packet_count_) {
			connection->Write(msg_buffer_, packet_size_);
		} else {
			++connected_counter_;
			if (connected_counter_ + connect_failed_counter_ == client_count_) {
				quit_ = true;
			}
		}
	}
}

i32 BenchClient::GetMessageSize(Net::Connection * connection) const {
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

void BenchClient::ProcessCommand(Net::Connection * connection) const {
	if (log_detail_) {
		PackHeader ph = {0};
		std::memcpy(&ph, connection->GetRecvData(), PACK_HEADER_LEN);
		const char * data = connection->GetRecvData() + PACK_HEADER_LEN;
		const int data_len = ph.data_len;
		Net::Log(Net::kLog, __FILE__, __LINE__, "Socket [", connection->GetConnectionId(), "] Package Length", data_len);
	}
}