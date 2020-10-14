#include "gtest/gtest.h"
#include "Address/SocketAddress.h"

class SocketAddressImplTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		EXPECT_EQ(0, uv_ip4_addr("192.168.1.100", 6789, &ipv4_addr_));
		EXPECT_EQ(0, uv_ip6_addr("fe80::6101:927f:1dde:cb33", 6789, &ipv6_addr_));
		ipv4_impl_ = new Net::IPv4SocketAddressImpl(&ipv4_addr_);
		ipv6_impl_ = new Net::IPv6SocketAddressImpl(&ipv6_addr_);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete ipv4_impl_;
		delete ipv6_impl_;
	}
	Net::IPv4SocketAddressImpl * ipv4_impl_;
	Net::IPv6SocketAddressImpl * ipv6_impl_;
	sockaddr_in ipv4_addr_;
	sockaddr_in6 ipv6_addr_;
};

TEST_F(SocketAddressImplTestSuite, ipv4_ctor) {
	Net::IPv4SocketAddressImpl ipv4;
	EXPECT_STREQ(*ipv4.Host().ToString(), "0.0.0.0");
	EXPECT_EQ(ipv4.Port(), 0);
	EXPECT_EQ(ipv4.Length(), sizeof(ipv4_addr_));
	EXPECT_LT(std::memcmp(ipv4.Addr(), ipv4_impl_->Addr(), ipv4.Length()), 0);
	EXPECT_EQ(ipv4.AF(), AF_INET);
	EXPECT_EQ(ipv4.Family(), Net::AddressFamily::IPv4);
	EXPECT_STREQ(*ipv4.ToString(), "0.0.0.0:0");
}

TEST_F(SocketAddressImplTestSuite, ipv6_ctor) {
	Net::IPv6SocketAddressImpl ipv6;
	EXPECT_STREQ(*ipv6.Host().ToString(), "::");
	EXPECT_EQ(ipv6.Port(), 0);
	EXPECT_EQ(ipv6.Length(), sizeof(ipv6_addr_));
	EXPECT_LT(std::memcmp(ipv6.Addr(), ipv6_impl_->Addr(), ipv6.Length()), 0);
	EXPECT_EQ(ipv6.AF(), AF_INET6);
	EXPECT_EQ(ipv6.Family(), Net::AddressFamily::IPv6);
	EXPECT_STREQ(*ipv6.ToString(), "[::]:0");
}

TEST_F(SocketAddressImplTestSuite, ipv4_display) {
	EXPECT_STREQ(*ipv4_impl_->Host().ToString(), "192.168.1.100");
	EXPECT_EQ(ipv4_impl_->Port(), 6789);
	EXPECT_EQ(ipv4_impl_->Length(), sizeof(ipv4_addr_));
	EXPECT_EQ(std::memcmp(ipv4_impl_->Addr(), &ipv4_addr_, ipv4_impl_->Length()), 0);
	EXPECT_EQ(ipv4_impl_->AF(), AF_INET);
	EXPECT_EQ(ipv4_impl_->Family(), Net::AddressFamily::IPv4);
	EXPECT_STREQ(*ipv4_impl_->ToString(), "192.168.1.100:6789");
}

TEST_F(SocketAddressImplTestSuite, ipv6_display) {
	EXPECT_STREQ(*ipv6_impl_->Host().ToString(), "fe80::6101:927f:1dde:cb33");
	EXPECT_EQ(ipv6_impl_->Port(), 6789);
	EXPECT_EQ(ipv6_impl_->Length(), sizeof(ipv6_addr_));
	EXPECT_EQ(std::memcmp(ipv6_impl_->Addr(), &ipv6_addr_, ipv6_impl_->Length()), 0);
	EXPECT_EQ(ipv6_impl_->AF(), AF_INET6);
	EXPECT_EQ(ipv6_impl_->Family(), Net::AddressFamily::IPv6);
	EXPECT_STREQ(*ipv6_impl_->ToString(), "[fe80::6101:927f:1dde:cb33]:6789");
}

class SocketAddressTestSuite : public SocketAddressImplTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		SocketAddressImplTestSuite::SetUp();
		ip_ = new Net::SocketAddress(Net::IPAddress("192.168.1.100"), 6789);
		ip6_ = new Net::SocketAddress(Net::IPAddress("fe80::6101:927f:1dde:cb33"), 6789);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete ip_;
		delete ip6_;
		SocketAddressImplTestSuite::TearDown();
	}
	Net::SocketAddress * ip_;
	Net::SocketAddress * ip6_;
};

TEST_F(SocketAddressTestSuite, ctor) {
	Net::SocketAddress ip;
	EXPECT_TRUE(ip.Host() == Net::IPAddress());
	EXPECT_EQ(ip.Port(), 0);
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_));
	EXPECT_LT(std::memcmp(ip.Addr(), &ipv4_addr_, ip.Length()), 0);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_STREQ(*ip.ToString(), "0.0.0.0:0");
}

TEST_F(SocketAddressTestSuite, ctor0) {
	Net::SocketAddress ip(6789);
	EXPECT_TRUE(ip.Host() == Net::IPAddress());
	EXPECT_EQ(ip.Port(), 6789);
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_));
	EXPECT_LT(std::memcmp(ip.Addr(), &ipv4_addr_, ip.Length()), 0);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_STREQ(*ip.ToString(), "0.0.0.0:6789");
}

TEST_F(SocketAddressTestSuite, ctor11) {
	Net::SocketAddress ip(" fe80::6101:927f:1dde:cb33 ", 6789);
	EXPECT_TRUE(ip.Host() == ip6_->Host());
	EXPECT_EQ(ip.Port(), 6789);
	EXPECT_EQ(ip.Length(), sizeof(ipv6_addr_));
	EXPECT_EQ(std::memcmp(ip.Addr(), &ipv6_addr_, ip.Length()), 0);
	EXPECT_EQ(ip.AF(), AF_INET6);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv6);
	EXPECT_STREQ(*ip.ToString(), "[fe80::6101:927f:1dde:cb33]:6789");
}

TEST_F(SocketAddressTestSuite, ctor1) {
	Net::SocketAddress ip("192.168.1.100", 6789);
	EXPECT_TRUE(ip.Host() == ip_->Host());
	EXPECT_EQ(ip.Port(), 6789);
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_));
	EXPECT_EQ(std::memcmp(ip.Addr(), &ipv4_addr_, ip.Length()), 0);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_STREQ(*ip.ToString(), "192.168.1.100:6789");
}

TEST_F(SocketAddressTestSuite, ctor2) {
	Net::SocketAddress ip(reinterpret_cast<sockaddr *>(&ipv4_addr_), sizeof(ipv4_addr_));
	EXPECT_TRUE(ip.Host() == ip_->Host());
	EXPECT_EQ(ip.Port(), ip_->Port());
	EXPECT_EQ(ip.Length(), ip_->Length());
	EXPECT_EQ(std::memcmp(ip.Addr(), ip_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.AF(), ip_->AF());
	EXPECT_EQ(ip.Family(), ip_->Family());
	EXPECT_STREQ(*ip.ToString(), "192.168.1.100:6789");
}

TEST_F(SocketAddressTestSuite, ctor3) {
	Net::SocketAddress ip(reinterpret_cast<sockaddr *>(&ipv6_addr_), sizeof(ipv6_addr_));
	EXPECT_TRUE(ip.Host() == ip6_->Host());
	EXPECT_EQ(ip.Port(), ip6_->Port());
	EXPECT_EQ(ip.Length(), ip6_->Length());
	EXPECT_EQ(std::memcmp(ip.Addr(), ip6_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.AF(), ip6_->AF());
	EXPECT_EQ(ip.Family(), ip6_->Family());
	EXPECT_STREQ(*ip.ToString(), "[fe80::6101:927f:1dde:cb33]:6789");
}

TEST_F(SocketAddressTestSuite, ctor4) {
	ipv4_addr_.sin_family = AF_UNIX;
	EXPECT_ANY_THROW(Net::SocketAddress ip(reinterpret_cast<sockaddr *>(&ipv4_addr_), sizeof(ipv4_addr_)));
}

TEST_F(SocketAddressTestSuite, ctor5) {
	Net::SocketAddress ip(*ip_);
	EXPECT_TRUE(ip.Host() == ip_->Host());
	EXPECT_EQ(ip.Port(), ip_->Port());
	EXPECT_EQ(ip.Length(), ip_->Length());
	EXPECT_EQ(std::memcmp(ip.Addr(), ip_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.AF(), ip_->AF());
	EXPECT_EQ(ip.Family(), ip_->Family());
	EXPECT_STREQ(*ip.ToString(), "192.168.1.100:6789");
}

TEST_F(SocketAddressTestSuite, ctor6) {
	Net::SocketAddress ip(*ip6_);
	EXPECT_TRUE(ip.Host() == ip6_->Host());
	EXPECT_EQ(ip.Port(), ip6_->Port());
	EXPECT_EQ(ip.Length(), ip6_->Length());
	EXPECT_EQ(std::memcmp(ip.Addr(), ip6_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.AF(), ip6_->AF());
	EXPECT_EQ(ip.Family(), ip6_->Family());
	EXPECT_STREQ(*ip.ToString(), "[fe80::6101:927f:1dde:cb33]:6789");
}

TEST_F(SocketAddressTestSuite, ctor7) {
	Net::SocketAddress ip;
	ip = *ip_;
	EXPECT_TRUE(ip.Host() == ip_->Host());
	EXPECT_EQ(ip.Port(), ip_->Port());
	EXPECT_EQ(ip.Length(), ip_->Length());
	EXPECT_EQ(std::memcmp(ip.Addr(), ip_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.AF(), ip_->AF());
	EXPECT_EQ(ip.Family(), ip_->Family());
	EXPECT_STREQ(*ip.ToString(), "192.168.1.100:6789");
}

TEST_F(SocketAddressTestSuite, ctor8) {
	Net::SocketAddress ip;
	ip = *ip6_;
	EXPECT_TRUE(ip.Host() == ip6_->Host());
	EXPECT_EQ(ip.Port(), ip6_->Port());
	EXPECT_EQ(ip.Length(), ip6_->Length());
	EXPECT_EQ(std::memcmp(ip.Addr(), ip6_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.AF(), ip6_->AF());
	EXPECT_EQ(ip.Family(), ip6_->Family());
	EXPECT_STREQ(*ip.ToString(), "[fe80::6101:927f:1dde:cb33]:6789");
}

TEST_F(SocketAddressTestSuite, ctor9) {
	Net::SocketAddress ip;
	ip = ip;
}

TEST_F(SocketAddressTestSuite, cmp) {
	EXPECT_TRUE(*ip_ != *ip6_);
}

TEST_F(SocketAddressTestSuite, cmp2) {
	Net::SocketAddress ip(ip_->Host(), 9999);
	EXPECT_TRUE(ip != *ip_);
}