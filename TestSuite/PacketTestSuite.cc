#include "gtest/gtest.h"
#include "PacketReader.hpp"
#include "Packet.hpp"

typedef struct test_struct {
	int a;
	u32 b;
	short c;
	u16 d;
	char e;
	u8 f;
} test_struct;

TEST(PacketTestSuite, use) {
	const char * null_str = nullptr;
	const char * hello_str = "hello world!";
	char str[128] = "I'm a superman, nice to meet you.";
	char buf[128] = {0};
	u8 a = 0x12;
	char b = 0x34;
	short c = 22222;
	u16 d = 55555;
	int e = 999999999;
	u32 f = 3333333333;
	i64 g = 0x4444444444444444;
	u64 h = 0xaaaaaaaaaaaaaaaa;

	Foundation::Packet writer, writer2(buf, 128);
	writer.SetAllocSize(1024);
	writer.WriteAtom<u8>(a);
	writer.WriteAtom<char>(b);
	writer.WriteAtom<short>(c);
	writer.WriteAtom<u16>(d);
	writer.WriteAtom<int>(e);
	writer.WriteAtom<u32>(f);
	writer.WriteAtom<i64>(g);
	writer.WriteAtom<u64>(h);
	test_struct ts = {1, 2, 3, 4, 5, 6};
	writer.WriteBinary(reinterpret_cast<const char *>(&ts), sizeof(ts));
	writer.WriteString(str);
	writer.WriteString(null_str);

	int size = sizeof(u8) + sizeof(char);
	size += sizeof(short) + sizeof(u16);
	size += sizeof(int) + sizeof(u32);
	size += sizeof(i64) + sizeof(u64);
	size += sizeof(ts);
	size += static_cast<int>(std::strlen(str)) + sizeof(u16) + 1;
	size += sizeof(u16) + 1;

	writer2 << a << b << c << d << e << f << g;
	writer2 << ts;
	writer2 << str;
	writer2 << hello_str;

	writer.SetPosition(size + 10);
	writer.SetPosition(1500);
	writer.SetPosition(0);

	EXPECT_EQ(0, writer.AdjustOffset(-1));
	EXPECT_EQ(10, writer.AdjustOffset(10));
	EXPECT_EQ(3000, writer.AdjustOffset(2990));

	writer.SetDataLength(4025);
	writer.SetDataLength(size);

	EXPECT_EQ(5049, writer.Reserve(5050));
	EXPECT_EQ(6074, writer.Reserve(100));

	Foundation::PacketReader reader, reader2(writer2.GetMemoryPtr(), writer2.GetDataLength());
	u8 a1;
	char b1;
	short c1;
	u16 d1;
	reader2 >> a1 >> b1 >> c1 >> d1;
	EXPECT_EQ(a1, 0x12);
	EXPECT_EQ(b1, 0x34);
	EXPECT_EQ(c1, 22222);
	EXPECT_EQ(d1, 55555);

	EXPECT_EQ(reader2.GetLength(), writer2.GetDataLength());
	EXPECT_EQ(reader2.GetDataLength(), writer2.GetDataLength());
	EXPECT_EQ(reader2.GetReadableLength(), reader2.GetDataLength() - 6);
	EXPECT_EQ(reader2.GetPosition(), 6);
	EXPECT_EQ(reader2.GetMemoryPtr(), writer2.GetMemoryPtr());
	EXPECT_EQ(reader2.GetOffsetPtr(), reader2.GetPositionPtr(6));

	int e1;
	u32 f1;
	i64 g1;
	test_struct ts1;
	char hello_str1[128] = { 0 };
	memset(&ts1, 0, sizeof(ts1));
	const char * str1;
	reader2 >> e1 >> f1 >> g1 >> ts1 >> str1;
	reader2.ReadString(hello_str1, 128);
	EXPECT_EQ(e1, 999999999);
	EXPECT_EQ(f1, 3333333333);
	EXPECT_EQ(g1, 0x4444444444444444);
	EXPECT_EQ(ts1.a, 1);
	EXPECT_EQ(ts1.b, 2);
	EXPECT_EQ(ts1.c, 3);
	EXPECT_EQ(ts1.d, 4);
	EXPECT_EQ(ts1.e, 5);
	EXPECT_EQ(ts1.f, 6);
	EXPECT_STREQ(str1, "I'm a superman, nice to meet you.");
	EXPECT_STREQ(hello_str1, "hello world!");

	int error1;
	u64 error2;
	reader2 >> error1 >> error2;
	EXPECT_EQ(0, error1);
	EXPECT_EQ(0, error2);

	EXPECT_EQ(reader2.SetPosition(1024), writer2.GetDataLength());
	EXPECT_EQ(reader2.SetPosition(0), 0);

	EXPECT_EQ(0, reader2.AdjustOffset(-1));
	EXPECT_EQ(writer2.GetDataLength(), reader2.AdjustOffset(1024));
	reader2.SetPosition(0);
	EXPECT_EQ(50, reader2.AdjustOffset(50));

	reader2.SetPosition(1024);
	const char * null_str1;
	reader2 >> null_str1;
	ASSERT_TRUE(null_str1 == nullptr);

	int size1 = sizeof(u8) + sizeof(char);
	size1 += sizeof(short) + sizeof(u16);
	size1 += sizeof(int) + sizeof(u32);
	size1 += sizeof(i64) + sizeof(test_struct);
	reader2.SetPosition(size1);
	int str_len = static_cast<int>(std::strlen("I'm a superman, nice to meet you."));
	char str2[128];
	reader2.ReadString(str2, str_len - 5);
	EXPECT_STREQ(str2, "I'm a superman, nice to mee");

	Foundation::PacketReader reader3(writer2.GetMemoryPtr(), writer2.GetDataLength() - 8);
	reader3.SetPosition(size1);
	reader3 >> null_str1;
	reader3.ReadString(str2, 128);
	EXPECT_STREQ(str2, "hello");
}

TEST(PacketTestSuite, alloc) {
	test_struct ts, ts1;
	ts.a = 1;
	ts.b = 2;
	ts.c = 0;
	ts.d = 4;
	ts.e = 0;
	ts.f = 6;
	ts1.a = 99;
	ts1.b = 99;
	ts1.c = 99;
	ts1.d = 99;
	ts1.e = 99;
	ts1.f = 99;
	Foundation::Packet packet;
	packet.WriteBinary(reinterpret_cast<const char *>(&ts), sizeof(ts));
	packet.WriteString("hello world");
	packet.SetPosition(0);
	packet >> ts1;
	const char * str1 = packet.ReadString();
	EXPECT_EQ(ts.a, ts1.a);
	EXPECT_EQ(ts.b, ts1.b);
	EXPECT_EQ(ts.c, ts1.c);
	EXPECT_EQ(ts.d, ts1.d);
	EXPECT_EQ(ts.e, ts1.e);
	EXPECT_EQ(ts.f, ts1.f);
	EXPECT_STREQ("hello world", str1);
}