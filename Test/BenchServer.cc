#include "BenchServer.h"
#include "Common/Allocator.h"

BenchServer::BenchServer(bool log_detail, bool auto_close, bool echo)
	: BenchCommon(log_detail), auto_close_(auto_close), echo_(echo)
	, server_(new Net::Server("BenchServer", reactor_, 65536, 65536)) {
	server_->SetNotification(this);
}

BenchServer::~BenchServer() {
	delete server_;
}

void BenchServer::Run(const std::string & address, i32 port) {
	server_->Listen(address, port);
	Poll();
}

void BenchServer::OnConnected(Net::Connection * connection) {
	++connected_counter_;
	if (disconnected_counter_ == connected_counter_ && auto_close_) {
		quit_ = true;
	}
}

void BenchServer::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
	if (disconnected_counter_ == connected_counter_ && auto_close_) {
		quit_ = true;
	}
}

void BenchServer::ProcessCommand(Net::Connection * connection, const i32 message_size) {
	BenchCommon::ProcessCommand(connection, message_size);
	if (echo_) {
		Send(connection, connection->GetRecvData(), message_size);
	}
}