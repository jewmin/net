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

#include "Address/SocketAddress.h"
#include "Common/Logger.h"

namespace Net {

SocketAddress::SocketAddress() {
	NewIPv4();
}

SocketAddress::SocketAddress(u16 port) {
	Init(IPAddress(), port);
}

SocketAddress::SocketAddress(const IPAddress & host, u16 port) {
	Init(host, port);
}

SocketAddress::SocketAddress(const struct sockaddr * addr, socklen_t length) {
	if (length == sizeof(struct sockaddr_in) && AF_INET == addr->sa_family) {
		NewIPv4(reinterpret_cast<const struct sockaddr_in *>(addr));
	} else if (length == sizeof(struct sockaddr_in6) && AF_INET6 == addr->sa_family) {
		NewIPv6(reinterpret_cast<const struct sockaddr_in6 *>(addr));
	} else {
		Log(kCrash, __FILE__, __LINE__, "invalid address length or family");
	}
}

SocketAddress::SocketAddress(const SocketAddress & rhs) {
	if (IPAddress::IPv4 == rhs.Family()) {
		NewIPv4(reinterpret_cast<const struct sockaddr_in *>(rhs.Addr()));
	} else if (IPAddress::IPv6 == rhs.Family()) {
		NewIPv6(reinterpret_cast<const struct sockaddr_in6 *>(rhs.Addr()));
	}
}

SocketAddress & SocketAddress::operator=(const SocketAddress & rhs) {
	if (this != &rhs) {
		Destroy();
		if (IPAddress::IPv4 == rhs.Family()) {
			NewIPv4(reinterpret_cast<const struct sockaddr_in *>(rhs.Addr()));
		} else if (IPAddress::IPv6 == rhs.Family()) {
			NewIPv6(reinterpret_cast<const struct sockaddr_in6 *>(rhs.Addr()));
		}
	}
	return *this;
}

SocketAddress::~SocketAddress() {
	Destroy();
}

bool SocketAddress::operator==(const SocketAddress & rhs) const {
	return Host() == rhs.Host() && Port() == rhs.Port();
}

bool SocketAddress::operator!=(const SocketAddress & rhs) const {
	return !(*this == rhs);
}

void SocketAddress::Init(const IPAddress & host, u16 port) {
	if (IPAddress::IPv4 == host.Family()) {
		struct sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = host.AF();
		std::memcpy(&addr.sin_addr, host.Addr(), host.Length());
		addr.sin_port = htons(port);
		NewIPv4(&addr);
	} else if (IPAddress::IPv6 == host.Family()) {
		struct sockaddr_in6 addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin6_family = host.AF();
		std::memcpy(&addr.sin6_addr, host.Addr(), host.Length());
		addr.sin6_scope_id = host.Scope();
		addr.sin6_port = htons(port);
		NewIPv6(&addr);
	}
}

void SocketAddress::NewIPv4() {
	new(Storage())IPv4SocketAddressImpl();
}

void SocketAddress::NewIPv4(const struct sockaddr_in * addr) {
	new(Storage())IPv4SocketAddressImpl(addr);
}

void SocketAddress::NewIPv6(const struct sockaddr_in6 * addr) {
	new(Storage())IPv6SocketAddressImpl(addr);
}

void SocketAddress::Destroy() {
	Impl()->~SocketAddressImpl();
}

}