#include "gtest/gtest.h"
#include "Address/IPAddress.h"

class IPAddressImplTestSuite : public testing::Test {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		EXPECT_EQ(0, uv_ip4_addr("192.168.1.100", 6789, &ipv4_addr_));
#ifdef _WIN32
		EXPECT_EQ(0, uv_ip6_addr("fe80::6101:927f:1dde:cb33%1", 6789, &ipv6_addr_));
#else
		EXPECT_EQ(0, uv_ip6_addr("fe80::6101:927f:1dde:cb33%lo", 6789, &ipv6_addr_));
#endif
		ipv4_impl_ = new Net::IPv4AddressImpl(&ipv4_addr_.sin_addr);
		ipv6_impl_ = new Net::IPv6AddressImpl(&ipv6_addr_.sin6_addr, ipv6_addr_.sin6_scope_id);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete ipv4_impl_;
		delete ipv6_impl_;
	}
	Net::IPv4AddressImpl * ipv4_impl_;
	Net::IPv6AddressImpl * ipv6_impl_;
	sockaddr_in ipv4_addr_;
	sockaddr_in6 ipv6_addr_;
};

TEST_F(IPAddressImplTestSuite, ipv4_ctor) {
	Net::IPv4AddressImpl ipv4_impl1, ipv4_impl2(*ipv4_impl_);
	ipv4_impl1 = *ipv4_impl_;
	EXPECT_STREQ(ipv4_impl1.ToString().c_str(), ipv4_impl2.ToString().c_str());
	EXPECT_EQ(std::memcmp(ipv4_impl1.Addr(), ipv4_impl2.Addr(), ipv4_impl1.Length()), 0);
	EXPECT_EQ(ipv4_impl1.Length(), ipv4_impl2.Length());
	EXPECT_EQ(ipv4_impl1.Family(), ipv4_impl2.Family());
	EXPECT_EQ(ipv4_impl1.AF(), ipv4_impl2.AF());
	EXPECT_EQ(ipv4_impl1.Scope(), ipv4_impl2.Scope());
}

TEST_F(IPAddressImplTestSuite, ipv6_ctor) {
	Net::IPv6AddressImpl ipv6_impl1, ipv6_impl2(*ipv6_impl_);
	ipv6_impl1 = *ipv6_impl_;
	EXPECT_STREQ(ipv6_impl1.ToString().c_str(), ipv6_impl2.ToString().c_str());
	EXPECT_EQ(std::memcmp(ipv6_impl1.Addr(), ipv6_impl2.Addr(), ipv6_impl1.Length()), 0);
	EXPECT_EQ(ipv6_impl1.Length(), ipv6_impl2.Length());
	EXPECT_EQ(ipv6_impl1.Family(), ipv6_impl2.Family());
	EXPECT_EQ(ipv6_impl1.AF(), ipv6_impl2.AF());
	EXPECT_EQ(ipv6_impl1.Scope(), ipv6_impl2.Scope());
}

TEST_F(IPAddressImplTestSuite, ipv4_cmp) {
	Net::IPv4AddressImpl ipv4_impl;
	EXPECT_TRUE(ipv4_impl != *ipv4_impl_);
}

TEST_F(IPAddressImplTestSuite, ipv6_cmp) {
	Net::IPv6AddressImpl ipv6_impl;
	EXPECT_TRUE(ipv6_impl != *ipv6_impl_);
}

TEST_F(IPAddressImplTestSuite, ipv4_get) {
	EXPECT_STREQ(ipv4_impl_->ToString().c_str(), "192.168.1.100");
	EXPECT_EQ(ipv4_impl_->Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_EQ(ipv4_impl_->Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ipv4_impl_->AF(), AF_INET);
	EXPECT_EQ((int)ipv4_impl_->Scope(), 0);
}

TEST_F(IPAddressImplTestSuite, ipv6_get) {
	EXPECT_STREQ(ipv6_impl_->ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_EQ(ipv6_impl_->Length(), sizeof(ipv6_addr_.sin6_addr));
	EXPECT_EQ(ipv6_impl_->Family(), Net::AddressFamily::IPv6);
	EXPECT_EQ(ipv6_impl_->AF(), AF_INET6);
	EXPECT_GE((int)ipv6_impl_->Scope(), 1);
}

TEST_F(IPAddressImplTestSuite, ipv4_parse) {
	EXPECT_STREQ(Net::IPv4AddressImpl::Parse("").ToString().c_str(), "0.0.0.0");
	EXPECT_STREQ(Net::IPv4AddressImpl::Parse("192.168.1.100").ToString().c_str(), "192.168.1.100");
	EXPECT_STREQ(Net::IPv4AddressImpl::Parse("error ip").ToString().c_str(), "0.0.0.0");
	EXPECT_STREQ(Net::IPv4AddressImpl::Parse("   192.168.1.100   ").ToString().c_str(), "0.0.0.0");
}

TEST_F(IPAddressImplTestSuite, ipv6_parse) {
	EXPECT_STREQ(Net::IPv6AddressImpl::Parse("").ToString().c_str(), "::");
	EXPECT_STREQ(Net::IPv6AddressImpl::Parse("fe80::6101:927f:1dde:cb33").ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_STREQ(Net::IPv6AddressImpl::Parse("error ip").ToString().c_str(), "::");
	EXPECT_STREQ(Net::IPv6AddressImpl::Parse("   fe80::6101:927f:1dde:cb33   ").ToString().c_str(), "::");
#ifdef _WIN32
	EXPECT_STREQ(Net::IPv6AddressImpl::Parse("fe80::6101:927f:1dde:cb33%1").ToString().c_str(), "fe80::6101:927f:1dde:cb33");
#else
	EXPECT_STREQ(Net::IPv6AddressImpl::Parse("fe80::6101:927f:1dde:cb33%lo").ToString().c_str(), "fe80::6101:927f:1dde:cb33");
#endif
}

class IPAddressTestSuite : public IPAddressImplTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		IPAddressImplTestSuite::SetUp();
		ip_ = new Net::IPAddress("192.168.1.100");
		ip6_ = new Net::IPAddress(ipv6_impl_->Addr(), ipv6_impl_->Length(), ipv6_impl_->Scope());
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete ip_;
		delete ip6_;
		IPAddressImplTestSuite::TearDown();
	}
	Net::IPAddress * ip_;
	Net::IPAddress * ip6_;
};

TEST_F(IPAddressTestSuite, ctor) {
	Net::IPAddress ip;
	EXPECT_STREQ(ip.ToString().c_str(), "0.0.0.0");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_LT(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor2) {
	Net::IPAddress ip("");
	EXPECT_STREQ(ip.ToString().c_str(), "0.0.0.0");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_LT(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor3) {
	Net::IPAddress ip("0.0.0.0");
	EXPECT_STREQ(ip.ToString().c_str(), "0.0.0.0");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_LT(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor4) {
	Net::IPAddress ip("::");
	EXPECT_STREQ(ip.ToString().c_str(), "::");
	EXPECT_EQ(ip.Length(), sizeof(ipv6_addr_.sin6_addr));
	EXPECT_LT(std::memcmp(ip.Addr(), ipv6_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv6);
	EXPECT_EQ(ip.AF(), AF_INET6);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor5) {
	Net::IPAddress ip("  192.168.1.100  ");
	EXPECT_STREQ(ip.ToString().c_str(), "192.168.1.100");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor6) {
#ifdef _WIN32
	Net::IPAddress ip("fe80::6101:927f:1dde:cb33%1");
#else
	Net::IPAddress ip("fe80::6101:927f:1dde:cb33%lo");
#endif
	EXPECT_STREQ(ip.ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_EQ(ip.Length(), sizeof(ipv6_addr_.sin6_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv6_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv6);
	EXPECT_EQ(ip.AF(), AF_INET6);
	EXPECT_GE((int)ip.Scope(), 1);
}

TEST_F(IPAddressTestSuite, ctor7) {
	EXPECT_ANY_THROW(Net::IPAddress ip("error ip"));
}

TEST_F(IPAddressTestSuite, ctor8) {
	Net::IPAddress ip(ipv4_impl_->Addr(), ipv4_impl_->Length(), ipv4_impl_->Scope());
	EXPECT_STREQ(ip.ToString().c_str(), "192.168.1.100");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor9) {
	Net::IPAddress ip(ipv6_impl_->Addr(), ipv6_impl_->Length(), ipv6_impl_->Scope());
	EXPECT_STREQ(ip.ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_EQ(ip.Length(), sizeof(ipv6_addr_.sin6_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv6_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv6);
	EXPECT_EQ(ip.AF(), AF_INET6);
	EXPECT_GE((int)ip.Scope(), 1);
}

TEST_F(IPAddressTestSuite, ctor10) {
	Net::IPAddress ip(*ip_);
	EXPECT_STREQ(ip.ToString().c_str(), "192.168.1.100");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, ctor11) {
	Net::IPAddress ip(*ip6_);
	EXPECT_STREQ(ip.ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_EQ(ip.Length(), sizeof(ipv6_addr_.sin6_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv6_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv6);
	EXPECT_EQ(ip.AF(), AF_INET6);
	EXPECT_GE((int)ip.Scope(), 1);
}

TEST_F(IPAddressTestSuite, ctor12) {
	EXPECT_ANY_THROW(Net::IPAddress ip(nullptr, 0));
}

TEST_F(IPAddressTestSuite, assign) {
	Net::IPAddress ip;
	ip = *ip_;
	EXPECT_STREQ(ip.ToString().c_str(), "192.168.1.100");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, assign2) {
	Net::IPAddress ip;
	ip = *ip6_;
	EXPECT_STREQ(ip.ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_EQ(ip.Length(), sizeof(ipv6_addr_.sin6_addr));
	EXPECT_EQ(std::memcmp(ip.Addr(), ipv6_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv6);
	EXPECT_EQ(ip.AF(), AF_INET6);
	EXPECT_GE((int)ip.Scope(), 1);
}

TEST_F(IPAddressTestSuite, assign3) {
	Net::IPAddress ip;
	ip = ip;
	EXPECT_STREQ(ip.ToString().c_str(), "0.0.0.0");
	EXPECT_EQ(ip.Length(), sizeof(ipv4_addr_.sin_addr));
	EXPECT_LT(std::memcmp(ip.Addr(), ipv4_impl_->Addr(), ip.Length()), 0);
	EXPECT_EQ(ip.Family(), Net::AddressFamily::IPv4);
	EXPECT_EQ(ip.AF(), AF_INET);
	EXPECT_EQ((int)ip.Scope(), 0);
}

TEST_F(IPAddressTestSuite, cmp) {
	Net::IPAddress ip(*ip_);
	Net::IPAddress ip6(ip6_->Addr(), ip6_->Length());
	EXPECT_TRUE(ip == *ip_);
	EXPECT_TRUE(ip6 != *ip6_);
	EXPECT_TRUE(ip_ != ip6_);
}

TEST_F(IPAddressTestSuite, parse) {
	*ip_ = Net::IPAddress::Parse("");
	EXPECT_STREQ(ip_->ToString().c_str(), "0.0.0.0");
	EXPECT_STREQ(Net::IPAddress::Parse("0.0.0.0").ToString().c_str(), "0.0.0.0");
	EXPECT_STREQ(Net::IPAddress::Parse("::").ToString().c_str(), "::");
	EXPECT_STREQ(Net::IPAddress::Parse("  192.168.1.100  ").ToString().c_str(), "192.168.1.100");
	*ip_ = Net::IPAddress::Parse("  fe80::6101:927f:1dde:cb33  ");
	EXPECT_STREQ(ip_->ToString().c_str(), "fe80::6101:927f:1dde:cb33");
	EXPECT_ANY_THROW(Net::IPAddress::Parse("error ip"));
}

TEST_F(IPAddressTestSuite, tryparse) {
	EXPECT_EQ(Net::IPAddress::TryParse("", *ip6_), true);
	EXPECT_STREQ(ip6_->ToString().c_str(), "0.0.0.0");

	EXPECT_EQ(Net::IPAddress::TryParse("0.0.0.0", *ip6_), true);
	EXPECT_STREQ(ip6_->ToString().c_str(), "0.0.0.0");

	EXPECT_EQ(Net::IPAddress::TryParse("::", *ip6_), true);
	EXPECT_STREQ(ip6_->ToString().c_str(), "::");

	EXPECT_EQ(Net::IPAddress::TryParse("  192.168.1.100  ", *ip6_), true);
	EXPECT_STREQ(ip6_->ToString().c_str(), "192.168.1.100");

	EXPECT_EQ(Net::IPAddress::TryParse("  fe80::6101:927f:1dde:cb33  ", *ip6_), true);
	EXPECT_STREQ(ip6_->ToString().c_str(), "fe80::6101:927f:1dde:cb33");

	EXPECT_EQ(Net::IPAddress::TryParse("error ip", *ip6_), false);
	EXPECT_STREQ(ip6_->ToString().c_str(), "fe80::6101:927f:1dde:cb33");
}
