#ifndef Benchmark_BenchServer_INCLUDED
#define Benchmark_BenchServer_INCLUDED

#include "BenchCommon.h"
#include "Interface/Server.h"

class BenchServer : public BenchCommon {
public:
	BenchServer(bool log_detail);
	virtual ~BenchServer();
	virtual void Run(const std::string & address, i32 port) override;
	using BenchCommon::ShowStatus;

private:
	Net::Server * server_;
};

#endif