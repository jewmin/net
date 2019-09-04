#include "gtest/gtest.h"
#include "Address/IPAddress.h"

using namespace Net;

TEST(IPAddressTestSuite, IPv4Construct) {
	sockaddr_in si;
	uv_ip4_addr("127.0.0.1", 7890, &si);

	IPAddress addr1;
	IPAddress addr2("159.75.63.78");
	IPAddress addr3(*reinterpret_cast<sockaddr *>(&si));
	IPAddress addr4(&si.sin_addr, sizeof(si.sin_addr));
	IPAddress addr5(addr1);
	IPAddress addr6;
	addr6 = addr2;
	addr6 = addr6;

	EXPECT_EQ(addr5, addr1);
	EXPECT_NE(addr2, addr3);
}

TEST(IPAddressTestSuite, IPv4) {
	std::string addresses[] = {"127.0.0.1", "192.168.1.1", "157.255.192.44", "255.255.255.255", "0.0.0.0", "0.0.0.1"};
	int size = sizeof(addresses) / sizeof(addresses[0]);
	for (int i = 0; i < size; i++) {
		IPAddress addr(addresses[i].c_str());
		EXPECT_EQ(addr.AF(), AF_INET);
		EXPECT_EQ(addr.Family(), IPAddress::IPv4);
		EXPECT_EQ(addr.Scope(), 0);
		EXPECT_STREQ(addr.ToString().c_str(), addresses[i].c_str());
	}
}

TEST(IPAddressTestSuite, IPv4Error) {
	std::string addresses[] = {"localhost", "www.google.com", "256.256.256.256", "1234567890", "1.2.3", "1.2.3.4.5", "127.0. 0.1", "-1.2.3.4", "172.16.01.06", " 127.0.0.1 "};
	int size = sizeof(addresses) / sizeof(addresses[0]);
	for (int i = 0; i < size; i++) {
		try {
			IPAddress addr(addresses[i].c_str());
		} catch (std::exception & e) {
			printf("IPAddressTestSuite - IPv4Error: %s\n", e.what());
		}
	}
}

TEST(IPAddressTestSuite, IPv6) {
	IPAddress addr1("fe80::a4f5:9de3:78bd:31d3%9"), addr2("::"), addr3(addr1.Addr(), addr1.Length(), addr1.Scope());
	IPAddress addr4("fe80::a4f5:9de3:78bd:31d3%eth0");
	EXPECT_EQ(addr1.AF(), AF_INET6);
	EXPECT_EQ(addr1.Family(), IPAddress::IPv6);
#ifdef _WIN32
	EXPECT_EQ(addr1.Scope(), 9);
#else
	EXPECT_EQ(addr4.Scope(), 9);
#endif
	EXPECT_STREQ(addr1.ToString().c_str(), "fe80::a4f5:9de3:78bd:31d3");
	EXPECT_NE(std::memcmp(addr1.Addr(), addr2.Addr(), addr1.Length()), 0);
	EXPECT_TRUE(addr1 != addr2);
}

TEST(IPAddressTestSuite, IPv6Error) {
	try {
		IPAddress addr1(nullptr, 0);
	} catch (std::exception & e) {
		printf("IPAddressTestSuite - IPv6Error: %s\n", e.what());
	}
}

TEST(IPAddressTestSuite, IP) {
	IPAddress addr1("127.0.0.1"), addr2("fe80::a4f5:9de3:78bd:31d3%9"), addr3("fe80::4435:43f0:11d2:4349%6");
	EXPECT_TRUE(addr1 != addr2);
	EXPECT_TRUE(addr1 != addr3);
	EXPECT_TRUE(addr2 != addr3);

	IPAddress addr = IPAddress::Parse("202.168.196.166");
	EXPECT_STREQ(addr.ToString().c_str(), "202.168.196.166");
	addr = IPAddress::Parse("::1");
	EXPECT_STREQ(addr.ToString().c_str(), "::1");

	EXPECT_EQ(IPAddress::TryParse(":::", addr), false);
	EXPECT_EQ(IPAddress::TryParse("::", addr), true);
	EXPECT_EQ(IPAddress::TryParse("", addr), true);
	EXPECT_EQ(IPAddress::TryParse("127.0.0.1", addr), true);
	EXPECT_EQ(IPAddress::TryParse("fe80::a4f5:9de3:78bd:31d3%9", addr), true);
}

TEST(IPAddressTestSuite, Copy) {
	IPAddress addr1("fe80::a4f5:9de3:78bd:31d3%9"), addr2("127.0.0.1");
	IPAddress addr;
	addr = addr1;
	addr = addr2;

	IPAddress addr3(addr1), addr4(addr2);
}

TEST(IPAddressTestSuite, Impl) {
	IPv4AddressImpl impl1;
	IPv4AddressImpl impl2(impl1);
	IPv4AddressImpl impl3;
	impl3 = IPv4AddressImpl::Parse("");

	IPv6AddressImpl impl61;
	IPv6AddressImpl impl62(impl61);
	IPv6AddressImpl impl63;
	impl63 = IPv6AddressImpl::Parse("");
}

TEST(IPAddressTestSuite, Clone) {
	IPv4AddressImpl impl;
	IPv6AddressImpl impl6;
	IPAddressImpl * ptr = impl.Clone();
	IPAddressImpl * ptr6 = impl6.Clone();
	EXPECT_EQ(ptr->AF(), AF_INET);
	EXPECT_EQ(ptr6->Family(), IPAddress::IPv6);

	delete ptr;
	delete ptr6;
}

TEST(IPAddressTestSuite, SocketAddr) {
	union {
		struct sockaddr_in v4;
		struct sockaddr addr;
	} addr4;
	uv_ip4_addr("127.0.0.1", 777, &addr4.v4);

	union {
		struct sockaddr_in6 v6;
		struct sockaddr addr;
	} addr6;
	uv_ip6_addr("fe80::a4f5:9de3:78bd:31d3%9", 888, &addr6.v6);

	IPAddress addr1(addr4.addr), addr2(addr6.addr);
	EXPECT_STREQ(addr1.ToString().c_str(), "127.0.0.1");
	EXPECT_STREQ(addr2.ToString().c_str(), "fe80::a4f5:9de3:78bd:31d3");
}

TEST(IPAddressTestSuite, ConstructError) {
	try {
		struct sockaddr addr;
		addr.sa_family = AF_UNIX;
		IPAddress address(addr);
	} catch (std::exception & e) {
		printf("IPAddressTestSuite - ConstructError: %s\n", e.what());
	}
}