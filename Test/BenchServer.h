#ifndef Benchmark_BenchServer_INCLUDED
#define Benchmark_BenchServer_INCLUDED

#include "BenchCommon.h"
#include "Interface/Server.h"

class BenchServer : public BenchCommon {
public:
	BenchServer(bool log_detail, bool auto_close);
	virtual ~BenchServer();
	virtual void Run(const std::string & address, i32 port) override;
	virtual void OnDisconnected(Net::Connection * connection, bool is_remote) override;
	using BenchCommon::ShowStatus;

protected:
	static void TimerCb(uv_timer_t * handle);

private:
	bool auto_close_;
	Net::Server * server_;
	uv_timer_t * timer_;
};

#endif