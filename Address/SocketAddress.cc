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

#include "SocketAddress.h"

Net::SocketAddress::SocketAddress() {
	NewIPv4();
}

Net::SocketAddress::SocketAddress(u16 port) {
	Init(IPAddress(), port);
}

Net::SocketAddress::SocketAddress(const IPAddress & hostAddress, u16 port) {
	Init(hostAddress, port);
}

Net::SocketAddress::SocketAddress(const std::string & hostAddress, u16 port) {
	Init(hostAddress, port);
}

Net::SocketAddress::SocketAddress(const sockaddr * addr, socklen_t length) {
	if (length == sizeof(struct sockaddr_in) && AF_INET == addr->sa_family) {
		NewIPv4(reinterpret_cast<const struct sockaddr_in *>(addr));
	} else if (length == sizeof(struct sockaddr_in6) && AF_INET6 == addr->sa_family) {
		NewIPv6(reinterpret_cast<const struct sockaddr_in6 *>(addr));
	} else {
		throw std::invalid_argument("SocketAddress(): invalid address length or family");
	}
}

Net::SocketAddress::SocketAddress(const SocketAddress & rhs) {
	if (IPv4 == rhs.Family()) {
		NewIPv4(reinterpret_cast<const struct sockaddr_in *>(rhs.Addr()));
	} else if (IPv6 == rhs.Family()) {
		NewIPv6(reinterpret_cast<const struct sockaddr_in6 *>(rhs.Addr()));
	}
}

Net::SocketAddress & Net::SocketAddress::operator=(const SocketAddress & rhs) {
	if (this != &rhs) {
		Destroy();
		if (IPv4 == rhs.Family()) {
			NewIPv4(reinterpret_cast<const struct sockaddr_in *>(rhs.Addr()));
		} else if (IPv6 == rhs.Family()) {
			NewIPv6(reinterpret_cast<const struct sockaddr_in6 *>(rhs.Addr()));
		}
	}

	return *this;
}

Net::SocketAddress::~SocketAddress() {
	Destroy();
}

bool Net::SocketAddress::operator==(const SocketAddress & socketAddress) const {
	return Host() == socketAddress.Host() && Port() == socketAddress.Port();
}

bool Net::SocketAddress::operator!=(const SocketAddress & socketAddress) const {
	return !(*this == socketAddress);
}

void Net::SocketAddress::Init(const IPAddress & hostAddress, u16 port) {
	if (IPv4 == hostAddress.Family()) {
		NewIPv4(hostAddress, port);
	} else if (IPv6 == hostAddress.Family()) {
		NewIPv6(hostAddress, port);
	}
}

void Net::SocketAddress::Init(const std::string & hostAddress, u16 port) {
	IPAddress ip;
	if (IPAddress::TryParse(hostAddress, ip)) {
		Init(ip, port);
	} else {
		throw std::invalid_argument(std::string("Init(): No address found for host - ") + hostAddress);
	}
}

void Net::SocketAddress::NewIPv4() {
	new(Storage())IPv4SocketAddressImpl();
}

void Net::SocketAddress::NewIPv4(const struct sockaddr_in * addr) {
	new(Storage())IPv4SocketAddressImpl(addr);
}

void Net::SocketAddress::NewIPv4(const IPAddress & hostAddress, u16 port) {
	new(Storage())IPv4SocketAddressImpl(hostAddress.Addr(), port);
}

void Net::SocketAddress::NewIPv6(const struct sockaddr_in6 * addr) {
	new(Storage())IPv6SocketAddressImpl(addr);
}

void Net::SocketAddress::NewIPv6(const IPAddress & hostAddress, u16 port) {
	new(Storage())IPv6SocketAddressImpl(hostAddress.Addr(), port, hostAddress.Scope());
}

void Net::SocketAddress::Destroy() {
	Impl()->~SocketAddressImpl();
}