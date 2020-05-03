#include "gtest/gtest.h"
#include "Common/Allocator.h"
#define private public
#define protected public
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
	throw std::runtime_error("LoggerTestSuite_DefaultAbort");
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

	EXPECT_EQ(false, jc_replace_abort(nullptr));
	EXPECT_EQ(true, jc_replace_abort(LoggerTestSuite_DefaultAbort));
}

TEST_F(LoggerTestSuite, item) {
	Net::LogItem item1;
	EXPECT_EQ((int)item1.tag_, 4);

	i64 num = 123;
	Net::LogItem item2((int)num);
	EXPECT_EQ((int)item2.tag_, 2);
	EXPECT_EQ(item2.data_.snum, num);

	Net::LogItem item3((long)num);
	EXPECT_EQ((int)item3.tag_, 2);
	EXPECT_EQ(item3.data_.snum, num);

	Net::LogItem item4((long long)num);
	EXPECT_EQ((int)item4.tag_, 2);
	EXPECT_EQ(item4.data_.snum, num);

	num = -123;
	Net::LogItem item5((int)num);
	EXPECT_EQ((int)item5.tag_, 2);
	EXPECT_EQ(item5.data_.snum, num);

	Net::LogItem item6((long)num);
	EXPECT_EQ((int)item6.tag_, 2);
	EXPECT_EQ(item6.data_.snum, num);

	Net::LogItem item7((long long)num);
	EXPECT_EQ((int)item7.tag_, 2);
	EXPECT_EQ(item7.data_.snum, num);

	u64 unum = 123;
	Net::LogItem item8((unsigned int)unum);
	EXPECT_EQ((int)item8.tag_, 3);
	EXPECT_EQ(item8.data_.unum, unum);

	Net::LogItem item9((unsigned long)unum);
	EXPECT_EQ((int)item9.tag_, 3);
	EXPECT_EQ(item9.data_.unum, unum);

	Net::LogItem item10((unsigned long long)unum);
	EXPECT_EQ((int)item10.tag_, 3);
	EXPECT_EQ(item10.data_.unum, unum);

	unum = -123;
	Net::LogItem item11((unsigned int)unum);
	EXPECT_EQ((int)item11.tag_, 3);
	EXPECT_EQ(item11.data_.unum, (unsigned int)unum);

	Net::LogItem item12((unsigned long)unum);
	EXPECT_EQ((int)item12.tag_, 3);
	EXPECT_EQ(item12.data_.unum, (unsigned long)unum);

	Net::LogItem item13((unsigned long long)unum);
	EXPECT_EQ((int)item13.tag_, 3);
	EXPECT_EQ(item13.data_.unum, (unsigned long long)unum);

	Net::LogItem item14("123");
	EXPECT_EQ((int)item14.tag_, 0);
	EXPECT_STREQ(item14.data_.str, "123");

	Net::LogItem item15("-123");
	EXPECT_EQ((int)item15.tag_, 0);
	EXPECT_STREQ(item15.data_.str, "-123");

	Net::LogItem item16("hello world");
	EXPECT_EQ((int)item16.tag_, 0);
	EXPECT_STREQ(item16.data_.str, "hello world");

	int * p = static_cast<int *>(jc_malloc(sizeof(int)));
	*p = 123;
	Net::LogItem item17(p);
	EXPECT_EQ((int)item17.tag_, 1);
	EXPECT_EQ(item17.data_.ptr, p);
	EXPECT_EQ(*(int*)item17.data_.ptr, 123);
	jc_free(p);
}

TEST_F(LoggerTestSuite, log) {
	Net::Log(Net::kLog, __FILE__, __LINE__, "这是一条调试日志", (void *)999, (void *)0xabcdef);
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
	EXPECT_ANY_THROW(Net::Log(Net::kCrash, "file", 100, "crash"));
}

#undef private
#undef protected