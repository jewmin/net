#include "BenchClient.h"
#include "Common/Allocator.h"

BenchClient::BenchClient(i32 client_count, i32 packet_count, i32 packet_size, bool log_detail)
	: BenchCommon(log_detail), connector_(new Net::SocketConnector(reactor_))
	, client_count_(client_count), packet_count_(packet_count), packet_size_(packet_size) {
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
	for (auto & it : clients_) {
		delete it;
	}
	delete connector_;
	jc_free(msg_buffer_);
}

void BenchClient::Run(const std::string & address, i32 port) {
	Net::Client * client = nullptr;
	for (i32 i = 0; i < client_count_; ++i) {
		std::stringstream ss;
		ss << "BenchClient_" << i;
		client = new Net::Client(ss.str(), reactor_, connector_, 65536, 65536);
		client->SetMgrId(i);
		client->SetNotification(this);
		client->Connect(address, port);
		clients_.push_back(client);
	}
	Poll();
}

void BenchClient::ShowStatus() {
	std::printf("计划连接数/成功连接数/失败连接数/关闭连接数 %d/%d/%d/%d\n", client_count_, connected_counter_, connect_failed_counter_, disconnected_counter_);
	std::printf("计划发包数/成功发包数 %d/%d\n", packet_count_ * client_count_, write_counter_);
	std::printf("计划发包大小/成功发包大小/成功收包大小 %lld/%lld/%lld\n", packet_count_ * static_cast<i64>(packet_size_) * client_count_, static_cast<i64>(packet_size_) * write_counter_, recv_packet_size_);
	std::printf("耗时(微秒) %lld\n", use_time_);
}

void BenchClient::OnConnected(Net::Connection * connection) {
	++connected_counter_;
	c2p_mapping_.insert({reinterpret_cast<intptr_t>(connection), 0});
	Send(connection, msg_buffer_, packet_size_);
}

void BenchClient::OnConnectFailed(Net::Connection * connection, i32 reason) {
	++connect_failed_counter_;
	if (disconnected_counter_ + connect_failed_counter_ >= client_count_) {
		quit_ = true;
	}
}

void BenchClient::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
	if (disconnected_counter_ + connect_failed_counter_ >= client_count_) {
		quit_ = true;
	}
}

void BenchClient::OnSomeDataSent(Net::Connection * connection) {
	auto it = c2p_mapping_.find(reinterpret_cast<intptr_t>(connection));
	if (it != c2p_mapping_.end()) {
		if (++it->second < packet_count_) {
			Send(connection, msg_buffer_, packet_size_);
		} else {
			connection->Shutdown(false);
		}
	}
}