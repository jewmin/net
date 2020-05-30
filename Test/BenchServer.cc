#include "BenchServer.h"

BenchServer::BenchServer(bool log_detail, bool auto_close)
	: BenchCommon(log_detail), auto_close_(auto_close)
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

void BenchServer::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
	if (disconnected_counter_ == connected_counter_ && auto_close_) {
		quit_ = true;
	}
}