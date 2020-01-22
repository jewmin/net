#include "gtest/gtest.h"
#include "Address/IPAddress.h"

using namespace Net;

TEST(IPAddressTestSuite, IPv4Construct) {
	sockaddr_in si;
	uv_ip4_addr("127.0.0.1", 7890, &si);

	IPAddress addr1;
	IPAddress addr2("159.75.63.78");
	IPAddress addr4(&si.sin_addr, sizeof(si.sin_addr));
	IPAddress addr5(addr1);
	IPAddress addr6;
	addr6 = addr2;
	addr6 = addr6;

	EXPECT_EQ(addr5, addr1);
	EXPECT_NE(addr2, addr4);
}

TEST(IPAddressTestSuite, IPv4) {
	
	std::string addresses[] = {"127.0.0.1", "192.168.1.1", "157.255.192.44", "255.255.255.255", "0.0.0.0", "0.0.0.1"};
	int size = sizeof(addresses) / sizeof(addresses[0]);
	for (int i = 0; i < size; i++) {
		IPAddress addr(addresses[i].c_str());
		EXPECT_EQ(addr.AF(), AF_INET);
		EXPECT_EQ(addr.Family(), IPAddress::IPv4);
		EXPECT_EQ(addr.Scope(), static_cast<u32>(0));
		EXPECT_EQ(addr.Length(), sizeof(struct in_addr));
		EXPECT_STREQ(addr.ToString().c_str(), addresses[i].c_str());
	}
}

TEST(IPAddressTestSuite, IPv6) {
	IPAddress addr1("fe80::a4f5:9de3:78bd:31d3%9"), addr2("::"), addr3(addr1.Addr(), addr1.Length(), addr1.Scope());
	IPAddress addr4("fe80::a4f5:9de3:78bd:31d3%eth0");
	IPAddress addr5("fe80::c4c3:ca33:130d:30a5%wlp4s0");
	EXPECT_EQ(addr1.AF(), AF_INET6);
	EXPECT_EQ(addr1.Family(), IPAddress::IPv6);
#ifdef _WIN32
	EXPECT_EQ(addr1.Scope(), 9);
#else
	printf("Ipv6: %s\n", addr5.ToString().c_str());
#endif
	EXPECT_STREQ(addr1.ToString().c_str(), "fe80::a4f5:9de3:78bd:31d3");
	EXPECT_NE(std::memcmp(addr1.Addr(), addr2.Addr(), addr1.Length()), 0);
	EXPECT_TRUE(addr1 != addr2);
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

//TEST(IPAddressTestSuite, Error) {
//	IPAddress ip("localhost");
//}

//TEST(IPAddressTestSuite, Error2) {
//	IPAddress ip(nullptr, 0);
//}
