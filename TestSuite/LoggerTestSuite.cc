#include "gtest/gtest.h"
#include "Logger.h"

void LoggerTestSuite_Log(const i8 * msg, i32 length) {
	static char buf[500];
	memcpy(buf, msg, length);
	buf[length] = 0;
	printf("LoggerTestSuite_Log: %s", buf);
}

class LoggerTestSuite : public testing::Test {
protected:
	// Sets up the test fixture.
	virtual void SetUp() {
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		jc_replace_logger(LoggerTestSuite_Log);
	}
};

TEST_F(LoggerTestSuite, use1) {
	void * ptr = reinterpret_cast<void *>(0x888888);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条调试日志");
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条信息日志", 123456789, 9999999999999);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条警告日志", -123456789, -888888888888888);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条信息日志", 987654321u, 55555555555555u);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条错误日志", "sksksk", ptr);
}

TEST_F(LoggerTestSuite, use2) {
	char buf[300], buf1[159];
	std::memset(buf, '.', sizeof(buf));
	std::memset(buf1, '.', sizeof(buf1));
	for (int i = 180; i < 200; ++i) {
		buf[i] = '0' + i - 180;
	}
	buf[sizeof(buf) - 2] = '\n';
	buf[sizeof(buf) - 1] = 0;
	buf1[sizeof(buf1) - 2] = '\n';
	buf1[sizeof(buf1) - 1] = 0;
	LoggerTestSuite_Log(buf, sizeof(buf));
	Net::Log(Net::kLog, __FILE__, __LINE__, buf);
	Net::Log(Net::kLog, __FILE__, __LINE__, buf1);
}

//TEST_F(LoggerTestSuite, use3) {
//	Net::Log(Net::kCrash, __FILE__, __LINE__, "crash");
//}