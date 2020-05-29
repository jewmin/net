#include "BenchServer.h"

BenchServer::BenchServer(bool log_detail)
	: BenchCommon(log_detail)
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