#include "gtest/gtest.h"
#include "Logger.h"

using namespace Foundation;

void TestLog(int level, const char * msg) {
	printf("这是自定义打印日志函数: %d %s\n", level, msg);
}

class LoggerTestSuite : public testing::Test {
protected:
	// Sets up the test fixture.
	virtual void SetUp() {
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		SetLogFunc(nullptr);
	}
};

TEST_F(LoggerTestSuite, use1) {
	LogDebug("这是一条调试日志");
	LogInfo("这是一条信息日志");
	LogWarn("这是一条警告日志");
	LogErr("这是一条错误日志");
}

TEST_F(LoggerTestSuite, use2) {
	SetLogFunc(TestLog);
	LogDebug("这是一条调试日志");
	LogInfo("这是一条信息日志");
	LogWarn("这是一条警告日志");
	LogErr("这是一条错误日志");
}

TEST_F(LoggerTestSuite, use3) {
	SetLogFunc(nullptr);
	LogDebug("这是一条调试日志");
	LogInfo("这是一条信息日志");
	LogWarn("这是一条警告日志");
	LogErr("这是一条错误日志");
}

TEST_F(LoggerTestSuite, use4) {
	char buf[1280];
	std::memset(buf, '.', sizeof(buf));
	for (int i = 1020; i < 1030; ++i) {
		buf[i] = '0' + i - 1020;
	}
	buf[1279] = 0;
	printf("原始字符串: %s\n", buf);
	LogInfo(buf);
	LogInfo("这是一条信息日志");
}