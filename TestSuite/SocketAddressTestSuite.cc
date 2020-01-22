#include "gtest/gtest.h"
#include "Address/SocketAddress.h"

using namespace Net;

TEST(SocketAddressTestSuite, Constructor) {
	SocketAddress address1;
	SocketAddress address2(7777);
	SocketAddress address3(IPAddress::Parse("127.0.0.1"), 8888);
	SocketAddress address4(IPAddress("fe80::e56a:23a:b5b8:e855%8"), 9999);

	struct sockaddr_in si4;
	uv_ip4_addr("192.168.245.81", 6666, &si4);
	struct sockaddr_in6 si6;
	uv_ip6_addr("fe80::4435:43f0:11d2:4349%6", 5555, &si6);
	SocketAddress address5(reinterpret_cast<struct sockaddr *>(&si4), sizeof(si4));
	SocketAddress address6(reinterpret_cast<struct sockaddr *>(&si6), sizeof(si6));

	SocketAddress address7(address5);
	SocketAddress address8(address6);

	address1 = address3;
	address1 = address4;

	EXPECT_EQ(address1, address4);
	EXPECT_NE(address1, address2);
	EXPECT_EQ(address3.AF(), AF_INET);
	EXPECT_EQ(address4.AF(), AF_INET6);
	EXPECT_EQ(address5.Family(), IPAddress::IPv4);
	EXPECT_EQ(address6.Family(), IPAddress::IPv6);
	EXPECT_EQ(address7.Host(), IPAddress("192.168.245.81"));
	EXPECT_EQ(address8.Host(), IPAddress("fe80::4435:43f0:11d2:4349%6"));
	EXPECT_EQ(address2.Port(), 7777);
	EXPECT_STREQ(address1.ToString().c_str(), "fe80::e56a:23a:b5b8:e855:9999");
	EXPECT_STREQ(address5.ToString().c_str(), "192.168.245.81:6666");
	EXPECT_EQ(address1.Length(), sizeof(si6));
	EXPECT_EQ(address3.Length(), sizeof(si4));
}

TEST(SocketAddressTestSuite, Impl) {
	IPv6SocketAddressImpl impl;
}

//TEST(SocketAddressTestSuite, Error) {
//	struct sockaddr_in si4;
//	struct sockaddr_in6 si6;
//	si6.sin6_family = AF_INET6;
//	SocketAddress address(reinterpret_cast<struct sockaddr *>(&si6), sizeof(si4));
//}