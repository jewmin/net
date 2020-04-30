#include "gtest/gtest.h"
#include "Common/Logger.h"

void LoggerTestSuite_ReplaceLog(const i8 * msg, i32 length) {
	static char buf[201];
	std::memcpy(buf, msg, length);
	buf[length] = 0;
	std::printf("LoggerTestSuite_Log: %s", buf);
}

void LoggerTestSuite_WriteMessage(const i8 * msg, i32 length) {
	write(STDOUT_FILENO, msg, length);
}

void LoggerTestSuite_DefaultAbort() {
	throw std::exception("abort");
}

class LoggerTestSuite : public testing::Test {
protected:
	// Sets up the test fixture.
	virtual void SetUp() {
		jc_replace_logger(LoggerTestSuite_ReplaceLog);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		jc_replace_logger(LoggerTestSuite_WriteMessage);
	}
};

TEST_F(LoggerTestSuite, replace) {
	EXPECT_EQ(false, jc_replace_logger(nullptr));
	EXPECT_EQ(true, jc_replace_logger(LoggerTestSuite_WriteMessage));

	EXPECT_EQ(true, jc_replace_abort(LoggerTestSuite_DefaultAbort));
}

TEST_F(LoggerTestSuite, item) {
	Net::LogItem item1;
	Net::LogItem item2((int)123);
	Net::LogItem item3((long)123);
	Net::LogItem item4((long long)123);
	Net::LogItem item5((int)-123);
	Net::LogItem item6((long)-123);
	Net::LogItem item7((long long)-123);
	Net::LogItem item8((unsigned int)123);
	Net::LogItem item9((unsigned long)123);
	Net::LogItem item10((unsigned long long)123);
	Net::LogItem item11((unsigned int)-123);
	Net::LogItem item12((unsigned long)-123);
	Net::LogItem item13((unsigned long long)-123);
	Net::LogItem item14("123");
	Net::LogItem item15("-123");
	Net::LogItem item16((void *)123);
	Net::LogItem item17((void *)0x123);
}

TEST_F(LoggerTestSuite, log) {
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条调试日志", (void *)123, (void *)0xabcdef);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条信息日志", (int)123456789, (long)9999999999999, (long long)888888888888888);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条警告日志", (int)-123456789, (long)-9999999999999, (long long)-888888888888888);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条详细日志", (unsigned int)123456789, (unsigned long)9999999999999, (unsigned long long)888888888888888);
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条致命日志", (unsigned int)-123456789, (unsigned long)-9999999999999, (unsigned long long)-888888888888888);
}

TEST_F(LoggerTestSuite, out_buffer) {
	char buf[300];
	std::memset(buf, '.', sizeof(buf));
	for (int i = 180; i < 220; ++i) {
		buf[i] = 'A' + i - 180;
	}
	buf[sizeof(buf) - 2] = '\n';
	buf[sizeof(buf) - 1] = 0;
	Net::Log(Net::kLog, __FILE__, __LINE__, buf);
	EXPECT_EQ(true, jc_replace_logger(LoggerTestSuite_WriteMessage));
	Net::Log(Net::kLog, __FILE__, __LINE__, buf);
	Net::Log(Net::kLog, __FILE__, __LINE__, buf, "error");
}

TEST_F(LoggerTestSuite, crash) {
	try {
		Net::Log(Net::kCrash, __FILE__, __LINE__, "crash");
	} catch (std::exception & e) {
		std::printf("LoggerTestSuite - crash: %s\n", e.what());
	}
}