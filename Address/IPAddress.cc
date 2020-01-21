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

namespace Net {

const AddressFamily::Family IPAddress::IPv4;
const AddressFamily::Family IPAddress::IPv6;

IPAddress::IPAddress() {
	NewIPv4();
}

IPAddress::IPAddress(const std::string & ip) {
	std::string trim_ip = Trim(ip);
	if (trim_ip.empty() || trim_ip == "0.0.0.0") {
		NewIPv4();
		return;
	}

	if (trim_ip == "::") {
		NewIPv6();
		return;
	}

	IPv4AddressImpl ipv4(IPv4AddressImpl::Parse(trim_ip));
	if (ipv4 != IPv4AddressImpl()) {
		NewIPv4(ipv4.Addr());
		return;
	}

	IPv6AddressImpl ipv6(IPv6AddressImpl::Parse(trim_ip));
	if (ipv6 != IPv6AddressImpl()) {
		NewIPv6(ipv6.Addr(), ipv6.Scope());
		return;
	}

	throw std::invalid_argument(std::string("IPAddress(): invalid or unsupported address - ") + ip);
}

IPAddress::IPAddress(const void * addr, socklen_t length, u32 scope) {
	if (sizeof(struct in_addr) == length) {
		NewIPv4(addr);
	} else if (sizeof(struct in6_addr) == length) {
		NewIPv6(addr, scope);
	} else {
		throw std::invalid_argument("IPAddress(): invalid address length");
	}
}

IPAddress::IPAddress(const IPAddress & rhs) {
	if (IPv4 == rhs.Family()) {
		NewIPv4(rhs.Addr());
	} else if (IPv6 == rhs.Family()) {
		NewIPv6(rhs.Addr(), rhs.Scope());
	}
}

IPAddress & IPAddress::operator=(const IPAddress & rhs) {
	if (this != &rhs) {
		Destroy();
		if (IPv4 == rhs.Family()) {
			NewIPv4(rhs.Addr());
		} else if (IPv6 == rhs.Family()) {
			NewIPv6(rhs.Addr(), rhs.Scope());
		}
	}
	return *this;
}

IPAddress::~IPAddress() {
	Destroy();
}

bool IPAddress::operator==(const IPAddress & rhs) const {
	return Length() == rhs.Length() && Scope() == rhs.Scope() && 0 == std::memcmp(Addr(), rhs.Addr(), Length());
}

bool IPAddress::operator!=(const IPAddress & rhs) const {
	return !(*this == rhs);
}

void IPAddress::NewIPv4() {
	new(Storage())IPv4AddressImpl();
}

void IPAddress::NewIPv4(const void * addr) {
	new(Storage())IPv4AddressImpl(addr);
}

void IPAddress::NewIPv6() {
	new(Storage())IPv6AddressImpl();
}

void IPAddress::NewIPv6(const void * addr, u32 scope) {
	new(Storage())IPv6AddressImpl(addr, scope);
}

void IPAddress::Destroy() {
	Impl()->~IPAddressImpl();
}

IPAddress IPAddress::Parse(const std::string & ip) {
	return IPAddress(ip);
}

bool IPAddress::TryParse(const std::string & ip, IPAddress & result) {
	std::string trim_ip = Trim(ip);
	if (trim_ip.empty() || trim_ip == "0.0.0.0") {
		result.NewIPv4();
		return true;
	}

	if (trim_ip == "::") {
		result.NewIPv6();
		return true;
	}

	IPv4AddressImpl ipv4(IPv4AddressImpl::Parse(trim_ip));
	if (ipv4 != IPv4AddressImpl()) {
		result.NewIPv4(ipv4.Addr());
		return true;
	}

	IPv6AddressImpl ipv6(IPv6AddressImpl::Parse(trim_ip));
	if (ipv6 != IPv6AddressImpl()) {
		result.NewIPv6(ipv6.Addr(), ipv6.Scope());
		return true;
	}

	return false;
}

}