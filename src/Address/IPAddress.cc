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

#include "Address/IPAddress.h"
#include "NetworkException.h"

namespace Net {

const AddressFamily::eFamily IPAddress::IPv4;
const AddressFamily::eFamily IPAddress::IPv6;

IPAddress::IPAddress() {
	NewIPv4();
}

IPAddress::IPAddress(const i8 * ip) {
	Common::SDString trim_ip = Common::SDString(ip).TrimStartAndEnd();
	if (trim_ip.Empty() || trim_ip == "0.0.0.0") {
		NewIPv4();
		return;
	}

	if (trim_ip == "::") {
		NewIPv6();
		return;
	}

	IPv4AddressImpl ipv4(IPv4AddressImpl::Parse(*trim_ip));
	if (ipv4 != IPv4AddressImpl()) {
		NewIPv4(ipv4.Addr());
		return;
	}

	IPv6AddressImpl ipv6(IPv6AddressImpl::Parse(*trim_ip));
	if (ipv6 != IPv6AddressImpl()) {
		NewIPv6(ipv6.Addr(), ipv6.Scope());
		return;
	}

	throw NetworkException(*Common::SDString::Format("invalid or unsupported address %s", *trim_ip));
}

IPAddress::IPAddress(const void * addr, socklen_t length, u32 scope) {
	if (sizeof(struct in_addr) == length) {
		NewIPv4(addr);
	} else if (sizeof(struct in6_addr) == length) {
		NewIPv6(addr, scope);
	} else {
		throw NetworkException("invalid address length");
	}
}

IPAddress::IPAddress(const IPAddress & other) {
	if (IPv4 == other.Family()) {
		NewIPv4(other.Addr());
	} else if (IPv6 == other.Family()) {
		NewIPv6(other.Addr(), other.Scope());
	}
}

IPAddress & IPAddress::operator=(const IPAddress & other) {
	if (this != std::addressof(other)) {
		Destroy();
		if (IPv4 == other.Family()) {
			NewIPv4(other.Addr());
		} else if (IPv6 == other.Family()) {
			NewIPv6(other.Addr(), other.Scope());
		}
	}
	return *this;
}

IPAddress::~IPAddress() {
	Destroy();
}

bool IPAddress::operator==(const IPAddress & other) const {
	return Length() == other.Length() && Scope() == other.Scope() && 0 == std::memcmp(Addr(), other.Addr(), Length());
}

bool IPAddress::operator!=(const IPAddress & other) const {
	return !(*this == other);
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

IPAddress IPAddress::Parse(const i8 * ip) {
	return IPAddress(ip);
}

bool IPAddress::TryParse(const i8 * ip, IPAddress & result) {
	Common::SDString trim_ip = Common::SDString(ip).TrimStartAndEnd();
	if (trim_ip.Empty() || trim_ip == "0.0.0.0") {
		result.NewIPv4();
		return true;
	}

	if (trim_ip == "::") {
		result.NewIPv6();
		return true;
	}

	IPv4AddressImpl ipv4(IPv4AddressImpl::Parse(*trim_ip));
	if (ipv4 != IPv4AddressImpl()) {
		result.NewIPv4(ipv4.Addr());
		return true;
	}

	IPv6AddressImpl ipv6(IPv6AddressImpl::Parse(*trim_ip));
	if (ipv6 != IPv6AddressImpl()) {
		result.NewIPv6(ipv6.Addr(), ipv6.Scope());
		return true;
	}

	return false;
}

}