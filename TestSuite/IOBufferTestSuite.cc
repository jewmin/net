#include "gtest/gtest.h"
#include "Common/BipBuffer.h"
#include "Common/StraightBuffer.h"

class MockIOBuffer : public Net::IOBuffer {
public:
	MockIOBuffer() : s(0) {}
	virtual ~MockIOBuffer() {}
	virtual i8 * GetReserveBlock(i32 want_size, i32 & actually_size) override {
		if (want_size > buffer_length_ - s) {
			actually_size = buffer_length_ - s;
		} else {
			actually_size = want_size;
		}
		return buffer_ + s;
	}
	virtual i8 * GetContiguousBlock(i32 & size) override {
		size = s;
		return buffer_;
	}
	virtual void Commit(i32 size) override {
		s += size;
	}
	virtual void DeCommit(i32 size) override {
		if (s > size) {
			s -= size;
			std::memmove(buffer_, buffer_ + size, s);
		} else {
			s = 0;
		}
	}
	virtual i32 GetCommitedSize() const override {
		return s;
	}
	int s;
};

class MockIOBufferTestSuite : public testing::Test {
protected:
	// Sets up the test fixture.
	virtual void SetUp() {
		io.Allocate(10);
		io.Write("0123456789", 10);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
	}

public:
	MockIOBuffer io;
};

TEST(IOBufferTestSuite, io_ctor) {
	MockIOBuffer io;
	io.Allocate(10);
	io.Allocate(20);

	MockIOBuffer io1;
}

TEST(IOBufferTestSuite, io_write) {
	MockIOBuffer io;
	io.Allocate(10);
	EXPECT_EQ(io.Write("abcd", 4), 4);
	EXPECT_EQ(io.Write("abcdefg", 7), 6);
	EXPECT_EQ(io.Write("abc", 3), UV_ENOBUFS);
}

TEST_F(MockIOBufferTestSuite, io_read) {
	char buffer[100];
	EXPECT_EQ(io.Read(buffer, sizeof(buffer)), 10);
	buffer[10] = 0;
	EXPECT_STREQ(buffer, "0123456789");
}

TEST_F(MockIOBufferTestSuite, io_read_repeat) {
	char buffer[8];
	EXPECT_EQ(io.Read(buffer, sizeof(buffer)), 8);
	buffer[7] = 0;
	EXPECT_STREQ(buffer, "0123456");
	EXPECT_EQ(io.Read(buffer, sizeof(buffer)), 2);
	buffer[2] = 0;
	EXPECT_STREQ(buffer, "89");
	EXPECT_EQ(io.Read(buffer, sizeof(buffer)), UV_ENOBUFS);
}

TEST(IOBufferTestSuite, straight) {
	Net::StraightBuffer io;
	io.Allocate(20);

	char buf[6] = { '1', '2', '3', '4', '5', '6' };
	char rbuf[21];
	int size;
	EXPECT_EQ(io.GetCommitedSize(), 0);
	char * block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(0);
	//1 --------------------
	block = io.GetContiguousBlock(size);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(size, 0);

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//2 123456--------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//3 123456123456--------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//4 123456123456123456--
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456123456123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_LT(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//5 12345612345612345612
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612345612345612");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(size, 0);

	block = io.GetContiguousBlock(size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, io.GetCommitedSize());

	io.DeCommit(12);
	EXPECT_EQ(8, io.GetCommitedSize());
	//6 ------------12345612
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//7 12345612123456------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612123456");

	io.DeCommit(12);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//8 ------------56------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "56");

	io.DeCommit(12);
	EXPECT_EQ(io.GetCommitedSize(), 0);
	//9 --------------------
}

TEST(IOBufferTestSuite, bip) {
	Net::BipBuffer io;
	io.Allocate(20);

	char buf[6] = { '1', '2', '3', '4', '5', '6' };
	char rbuf[21];
	int size;
	char * block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(0);
	//1 --------------------
	block = io.GetContiguousBlock(size);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(size, 0);

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//2 123456--------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//3 123456123456--------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//4 123456123456123456--
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456123456123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_LT(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//5 12345612345612345612
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612345612345612");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(size, 0);

	block = io.GetContiguousBlock(size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, io.GetCommitedSize());

	io.DeCommit(12);
	EXPECT_EQ(8, io.GetCommitedSize());
	//6 ------------12345612
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//7 123456------12345612
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612");

	block = io.GetContiguousBlock(size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_LT(size, io.GetCommitedSize());

	io.DeCommit(12);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//8 123456--------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456");

	io.DeCommit(12);
	EXPECT_EQ(io.GetCommitedSize(), 0);
	//9 --------------------

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//10 123456--------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//11 123456123456--------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "123456123456");

	io.DeCommit(8);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//12 --------3456--------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "3456");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//13 --------3456123456--
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "3456123456");

	block = io.GetReserveBlock(1, size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, 1);
	std::memcpy(block, "9", size);
	io.Commit(sizeof(buf));
	//14 --------34561234569-
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "34561234569");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//15 123456--34561234569-
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "34561234569");

	block = io.GetReserveBlock(sizeof(buf), size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_LT(size, sizeof(buf));
	std::memcpy(block, buf, size);
	io.Commit(sizeof(buf));
	//16 1234561234561234569-
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "34561234569");

	io.DeCommit(20);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//17 12345612------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345612");

	io.DeCommit(7);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//18 -------2------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "2");

	block = io.GetReserveBlock(7, size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, 7);
	std::memcpy(block, "1234567", size);
	io.Commit(7);
	//19 -------21234567-----
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "21234567");

	block = io.GetReserveBlock(7, size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, 7);
	std::memcpy(block, "1234567", size);
	io.Commit(7);
	//20 123456721234567-----
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "21234567");

	block = io.GetReserveBlock(7, size);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(size, 0);

	io.DeCommit(20);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//21 1234567-------------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "1234567");

	block = io.GetReserveBlock(7, size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_EQ(size, 7);
	std::memcpy(block, "1234567", size);
	io.Commit(7);
	//22 12345671234567------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "12345671234567");

	io.DeCommit(7);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//23 -------1234567------

	block = io.GetReserveBlock(8, size);
	EXPECT_FALSE(block == nullptr);
	EXPECT_LT(size, 8);
	std::memcpy(block, "12345678", size);
	io.Commit(8);
	//24 12345671234567------
	block = io.GetContiguousBlock(size);
	std::memcpy(rbuf, block, size);
	rbuf[size] = 0;
	EXPECT_STREQ(rbuf, "1234567");

	io.DeCommit(20);
	EXPECT_GT(io.GetCommitedSize(), 0);
	//25 1234567-------------

	io.DeCommit(20);
	EXPECT_EQ(io.GetCommitedSize(), 0);
	//26 --------------------
}