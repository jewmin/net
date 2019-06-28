/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "IPAddress.h"

Net::IPAddress::IPAddress() {
	NewIPv4();
}

Net::IPAddress::IPAddress(AddressFamily::Family family) {
	if (IPv4 == family) {
		NewIPv4();
	} else if (IPv6 == family) {
		NewIPv6();
	} else {
		throw std::invalid_argument("Invalid or unsupported address family passed to IPAddress()");
	}
}

Net::IPAddress::IPAddress(const std::string & addr) {
	if (addr.empty() || Trim(addr) == "0.0.0.0") {
		NewIPv4();
		return;
	}

	if (Trim(addr) == "::") {
		NewIPv6();
		return;
	}

	IPv4AddressImpl addr4(IPv4AddressImpl::Parse(addr));
	if (addr4 != IPv4AddressImpl()) {
		NewIPv4(addr4.Addr());
		return;
	}

	IPv6AddressImpl addr6(IPv6AddressImpl::Parse(addr));
	if (addr6 != IPv6AddressImpl()) {
		NewIPv6(addr6.Addr(), addr6.Scope());
		return;
	}

	std::stringstream ss;
	ss << "Invalid or unsupported address passed to IPAddress(): " << addr;
	throw std::invalid_argument(ss.str());
}

Net::IPAddress::IPAddress(const std::string & addr, AddressFamily::Family family) {
	if (IPv4 == family) {
		IPv4AddressImpl addr4(IPv4AddressImpl::Parse(addr));
		NewIPv4(addr4.Addr());
	} else if (IPv6 == family) {
		IPv6AddressImpl addr6(IPv6AddressImpl::Parse(addr));
		NewIPv6(addr6.Addr(), addr6.Scope());
	} else {
		throw std::invalid_argument("Invalid or unsupported address family passed to IPAddress()");
	}
}

Net::IPAddress::IPAddress(const void * addr, socklen_t length, u32 scope) {
	if (sizeof(struct in_addr) == length) {
		NewIPv4(addr);
	} else if (sizeof(struct in6_addr) == length) {
		NewIPv6(addr, scope);
	} else {
		throw std::invalid_argument("Invalid address length passed to IPAddress()");
	}
}

Net::IPAddress::IPAddress(const struct sockaddr & sockaddr) {
	u16 family = sockaddr.sa_family;
	if (AF_INET == family) {
		NewIPv4(&reinterpret_cast<const struct sockaddr_in *>(&sockaddr)->sin_addr);
	} else if (AF_INET6 == family) {
		NewIPv6(&reinterpret_cast<const struct sockaddr_in6 *>(&sockaddr)->sin6_addr, reinterpret_cast<const struct sockaddr_in6 *>(&sockaddr)->sin6_scope_id);
	} else {
		throw std::invalid_argument("Invalid or unsupported address family passed to IPAddress()");
	}
}

Net::IPAddress::IPAddress(const IPAddress & rhs) {
	if (IPv4 == rhs.Family()) {
		NewIPv4(rhs.Addr());
	} else if (IPv6 == rhs.Family()) {
		NewIPv6(rhs.Addr(), rhs.Scope());
	}
}

Net::IPAddress & Net::IPAddress::operator=(const IPAddress & rhs) {
	if (this != &rhs) {
		Destroy();
		if (IPv4 == rhs.Family()) {
			NewIPv4(rhs.Addr());
		} else if (IPv6 == rhs.Family()) {
			NewIPv6(rhs.Addr(), rhs.Scope());
		} else {
			throw std::invalid_argument("Invalid or unsupported address family");
		}
	}

	return *this;
}

Net::IPAddress::~IPAddress() {
	Destroy();
}

Net::IPAddress Net::IPAddress::Parse(const std::string & addr) {
	return IPAddress(addr);
}

bool Net::IPAddress::TryParse(const std::string & addr, IPAddress & result) {
	if (addr.empty() || Trim(addr) == "0.0.0.0") {
		result.NewIPv4();
		return true;
	}

	if (Trim(addr) == "::") {
		result.NewIPv6();
		return true;
	}

	IPv4AddressImpl addr4(IPv4AddressImpl::Parse(addr));
	if (addr4 != IPv4AddressImpl()) {
		result.NewIPv4(addr4.Addr());
		return true;
	}

	IPv6AddressImpl addr6(IPv6AddressImpl::Parse(addr));
	if (addr6 != IPv6AddressImpl()) {
		result.NewIPv6(addr6.Addr(), addr6.Scope());
		return true;
	}

	return false;
}

bool Net::IPAddress::operator==(const IPAddress & addr) const {
	if (Length() != addr.Length()) {
		return false;
	}

	if (Scope() != addr.Scope()) {
		return false;
	}

	return 0 == std::memcmp(Addr(), addr.Addr(), Length());
}

bool Net::IPAddress::operator!=(const IPAddress & addr) const {
	return !(*this == addr);
}

void Net::IPAddress::NewIPv4() {
	new(Storage())IPv4AddressImpl();
}

void Net::IPAddress::NewIPv4(const void * host) {
	new(Storage())IPv4AddressImpl(host);
}

void Net::IPAddress::NewIPv6() {
	new(Storage())IPv6AddressImpl();
}

void Net::IPAddress::NewIPv6(const void * host, u32 scope) {
	new(Storage())IPv6AddressImpl(host, scope);
}

void Net::IPAddress::Destroy() {
	Impl()->~IPAddressImpl();
}