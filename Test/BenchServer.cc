#include "BenchServer.h"
#include "Common/Allocator.h"

BenchServer::BenchServer(bool log_detail, bool auto_close)
	: BenchCommon(log_detail), auto_close_(auto_close)
	, server_(new Net::Server("BenchServer", reactor_, 65536, 65536))
	, timer_(static_cast<uv_timer_t *>(jc_malloc(sizeof(uv_timer_t)))) {
	uv_timer_init(reactor_->GetUvLoop(), timer_);
	timer_->data = this;
	server_->SetNotification(this);
}

BenchServer::~BenchServer() {
	uv_close(reinterpret_cast<uv_handle_t *>(timer_), BenchCommon::CloseCb);
	delete server_;
}

void BenchServer::Run(const std::string & address, i32 port) {
	uv_timer_start(timer_, TimerCb, 3000, 3000);
	server_->Listen(address, port);
	Poll();
}

void BenchServer::OnDisconnected(Net::Connection * connection, bool is_remote) {
	++disconnected_counter_;
	if (disconnected_counter_ == connected_counter_ && auto_close_) {
		quit_ = true;
	}
}

void BenchServer::TimerCb(uv_timer_t * handle) {
	BenchServer * server = static_cast<BenchServer *>(handle->data);
	if (server) {
		server->ShowStatus();
	}
}