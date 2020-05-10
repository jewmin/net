#include "gtest/gtest.h"
#include "Common/IOBuffer.h"
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
	virtual i32 GetFreeSize() const override {
		return buffer_length_ - s;
	}
	i8 * GetBuffer() const {
		return buffer_;
	}
	i32 GetBufferLength() const {
		return buffer_length_;
	}
	void Release() {
		DeAllocate();
	}

	int s;
};

TEST(IOBufferTestSuite, io_ctor) {
	MockIOBuffer io;
	EXPECT_TRUE(io.GetBuffer() == nullptr);
	EXPECT_EQ(io.GetBufferLength(), 0);
}

TEST(IOBufferTestSuite, io_ctor2) {
	MockIOBuffer io;
	io.Allocate(10);
	EXPECT_TRUE(io.GetBuffer() != nullptr);
	EXPECT_EQ(io.GetBufferLength(), 10);
	io.Release();
}

TEST(IOBufferTestSuite, io_ctor3) {
	MockIOBuffer io;
	io.Allocate(10);
	io.Allocate(20);
	EXPECT_TRUE(io.GetBuffer() != nullptr);
	EXPECT_EQ(io.GetBufferLength(), 10);
}

class MockIOBufferTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		io_.Allocate(10);
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		io_.Release();
	}

	MockIOBuffer io_;
};

TEST_F(MockIOBufferTestSuite, io_write) {
	EXPECT_EQ(io_.Write("abcd", 4), 4);
	EXPECT_EQ(io_.Write("abcdefg", 7), 6);
	EXPECT_EQ(io_.Write("abc", 3), UV_ENOBUFS);
}

class MockReadIOBufferTestSuite : public MockIOBufferTestSuite {
public:
	MockReadIOBufferTestSuite() {
		std::memset(buffer_, 0, sizeof(buffer_));
		std::memset(small_buffer_, 0, sizeof(small_buffer_));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		MockIOBufferTestSuite::SetUp();
		io_.Write("abcdefghijklmnopqrstuvwxyz", static_cast<i32>(std::strlen("abcdefghijklmnopqrstuvwxyz")));
		std::memset(buffer_, 0, sizeof(buffer_));
		std::memset(small_buffer_, 0, sizeof(small_buffer_));
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		MockIOBufferTestSuite::TearDown();
	}

	i8 buffer_[20];
	i8 small_buffer_[8];
};

TEST_F(MockReadIOBufferTestSuite, io_read) {
	EXPECT_EQ(io_.Read(buffer_, sizeof(buffer_)), 10);
	EXPECT_STREQ(buffer_, "abcdefghij");
}

TEST_F(MockReadIOBufferTestSuite, io_read_repeat) {
	EXPECT_EQ(io_.Read(small_buffer_, sizeof(small_buffer_)), 8);
	small_buffer_[7] = 0;
	EXPECT_STREQ(small_buffer_, "abcdefg");
	EXPECT_EQ(io_.Read(small_buffer_, sizeof(small_buffer_)), 2);
	small_buffer_[2] = 0;
	EXPECT_STREQ(small_buffer_, "ij");
	EXPECT_EQ(io_.Read(small_buffer_, sizeof(small_buffer_)), 0);
}

class MockStraightBufferTestSuite : public MockReadIOBufferTestSuite {
public:
	MockStraightBufferTestSuite() : actually_size_(0), sb_(nullptr) {
		std::memset(read_buffer_, 0, sizeof(read_buffer_));
	}
	// Sets up the test fixture.
	virtual void SetUp() {
		MockReadIOBufferTestSuite::SetUp();
		std::memset(read_buffer_, 0, sizeof(read_buffer_));
		actually_size_ = 0;
		sb_ = new Net::StraightBuffer();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete sb_;
		MockReadIOBufferTestSuite::TearDown();
	}

	void WriteReadBuffer(i8 * data, i32 len) {
		if (data > 0) {
			std::memcpy(read_buffer_, data, len);
		}
		read_buffer_[len] = 0;
	}

	i8 read_buffer_[50];
	i32 actually_size_;
	Net::StraightBuffer * sb_;
};

TEST_F(MockStraightBufferTestSuite, ctor) {
	EXPECT_TRUE(sb_->GetReserveBlock(10, actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	EXPECT_TRUE(sb_->GetContiguousBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	sb_->Commit(10);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 0);

	sb_->Commit(0);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 0);

	EXPECT_ANY_THROW(sb_->DeCommit(-10));
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 0);

	sb_->DeCommit(0);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 0);

	sb_->DeCommit(10);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 0);
}

TEST_F(MockStraightBufferTestSuite, alloc) {
	sb_->Allocate(20);

	EXPECT_TRUE(sb_->GetReserveBlock(10, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 10);
	EXPECT_TRUE(sb_->GetReserveBlock(20, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);
	EXPECT_TRUE(sb_->GetReserveBlock(30, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);

	EXPECT_TRUE(sb_->GetContiguousBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	sb_->Commit(0);

	sb_->Commit(10);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 20);

	sb_->Commit(0);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 20);

	EXPECT_ANY_THROW(sb_->DeCommit(-10));
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 20);

	sb_->DeCommit(0);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 20);

	sb_->DeCommit(10);
	EXPECT_EQ(sb_->GetCommitedSize(), 0);
	EXPECT_EQ(sb_->GetFreeSize(), 20);
}

TEST_F(MockStraightBufferTestSuite, write) {
	char * block = nullptr;
	char * cblock = nullptr;
	const char * content = "123456";
	i32 content_len = static_cast<i32>(std::strlen(content));
	sb_->Allocate(20);

	block = sb_->GetReserveBlock(content_len, actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------

	block = sb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 14);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456");
	// -----------------------------------------
	// |1|2|3|4|5|6| | | | | | | | | | | | | | |
	// -----------------------------------------

	block = sb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 8);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6| | | | | | | | |
	// -----------------------------------------

	block = sb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 2);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456123456123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6|1|2|3|4|5|6| | |
	// -----------------------------------------

	block = sb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 2);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612345612345612");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6|1|2|3|4|5|6|1|2|
	// -----------------------------------------

	block = sb_->GetReserveBlock(content_len, actually_size_);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(actually_size_, 0);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612345612345612");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|3|4|5|6|1|2|3|4|5|6|1|2|
	// -----------------------------------------

	sb_->DeCommit(8);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 8);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "345612345612");
	// -----------------------------------------
	// | | | | | | | | |3|4|5|6|1|2|3|4|5|6|1|2|
	// -----------------------------------------

	sb_->DeCommit(4);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 12);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612");
	// -----------------------------------------
	// | | | | | | | | | | | | |1|2|3|4|5|6|1|2|
	// -----------------------------------------
	
	block = sb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	sb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 6);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345612123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|1|2|1|2|3|4|5|6| | | | | | |
	// -----------------------------------------

	sb_->DeCommit(50);
	cblock = sb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(sb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(sb_->GetFreeSize(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------
}

class MockBipBufferTestSuite : public MockStraightBufferTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		MockStraightBufferTestSuite::SetUp();
		bb_ = new Net::BipBuffer();
	}

	// Tears down the test fixture.
	virtual void TearDown() {
		delete bb_;
		MockStraightBufferTestSuite::TearDown();
	}

	Net::BipBuffer * bb_;
};

TEST_F(MockBipBufferTestSuite, ctor) {
	EXPECT_TRUE(bb_->GetReserveBlock(10, actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	EXPECT_TRUE(bb_->GetContiguousBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	bb_->Commit(10);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 0);

	bb_->Commit(0);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 0);

	EXPECT_ANY_THROW(bb_->DeCommit(-10));
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 0);

	bb_->DeCommit(0);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 0);

	bb_->DeCommit(10);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 0);
}

TEST_F(MockBipBufferTestSuite, alloc) {
	bb_->Allocate(20);

	EXPECT_TRUE(bb_->GetReserveBlock(10, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 10);
	EXPECT_TRUE(bb_->GetReserveBlock(20, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);
	EXPECT_TRUE(bb_->GetReserveBlock(30, actually_size_) != nullptr);
	EXPECT_EQ(actually_size_, 20);

	EXPECT_TRUE(bb_->GetContiguousBlock(actually_size_) == nullptr);
	EXPECT_EQ(actually_size_, 0);

	bb_->Commit(0);

	bb_->Commit(10);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 20);

	bb_->Commit(0);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 20);

	EXPECT_ANY_THROW(bb_->DeCommit(-10));
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 20);

	bb_->DeCommit(0);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 20);

	bb_->DeCommit(10);
	EXPECT_EQ(bb_->GetCommitedSize(), 0);
	EXPECT_EQ(bb_->GetFreeSize(), 20);
}

TEST_F(MockBipBufferTestSuite, write) {
	char * block = nullptr;
	char * cblock = nullptr;
	const char * content = "1234567";
	i32 content_len = static_cast<i32>(std::strlen(content));
	bb_->Allocate(20);

	block = bb_->GetReserveBlock(content_len, actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 13);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "1234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | | | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 6);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345671234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7| | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 6);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345671234567123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7|1|2|3|4|5|6|
	// -----------------------------------------

	bb_->DeCommit(10);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 10);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4567123456");
	// -----------------------------------------
	// | | | | | | | | | | |4|5|6|7|1|2|3|4|5|6|
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 3);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4567123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | |4|5|6|7|1|2|3|4|5|6|
	// -----------------------------------------

	bb_->DeCommit(4);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 7);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | |1|2|3|4|5|6|
	// -----------------------------------------

	bb_->DeCommit(40);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 13);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "1234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | | | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(4, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 4);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 9);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345671234");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4| | | | | | | | | |
	// -----------------------------------------

	bb_->DeCommit(10);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 10);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4");
	// -----------------------------------------
	// | | | | | | | | | | |4| | | | | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(14, actually_size_);
	std::memcpy(block, "1234567890", actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 10);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0|4| | | | | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	EXPECT_TRUE(block == nullptr);
	EXPECT_EQ(actually_size_, 0);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "4");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0|4| | | | | | | | | |
	// -----------------------------------------

	bb_->DeCommit(10);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 10);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "1234567890");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0| | | | | | | | | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 3);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "12345678901234567");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7| | | |
	// -----------------------------------------

	bb_->DeCommit(15);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 15);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | |6|7| | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 8);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// |1|2|3|4|5|6|7| | | | | | | | |6|7| | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 7);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 1);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7| |6|7| | | |
	// -----------------------------------------

	block = bb_->GetReserveBlock(content_len, actually_size_);
	std::memcpy(block, content, actually_size_);
	bb_->Commit(actually_size_);
	EXPECT_TRUE(block != nullptr);
	EXPECT_EQ(actually_size_, 1);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_GT(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 0);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "67");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7|1|6|7| | | |
	// -----------------------------------------

	bb_->DeCommit(20);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock != nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 5);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "123456712345671");
	// -----------------------------------------
	// |1|2|3|4|5|6|7|1|2|3|4|5|6|7|1| | | | | |
	// -----------------------------------------

	bb_->DeCommit(20);
	cblock = bb_->GetContiguousBlock(actually_size_);
	EXPECT_TRUE(cblock == nullptr);
	EXPECT_EQ(bb_->GetCommitedSize(), actually_size_);
	EXPECT_EQ(bb_->GetFreeSize(), 20);
	WriteReadBuffer(cblock, actually_size_);
	EXPECT_STREQ(read_buffer_, "");
	// -----------------------------------------
	// | | | | | | | | | | | | | | | | | | | | |
	// -----------------------------------------
}