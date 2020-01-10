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

#include "IPAddressImpl.h"

namespace Net {

IPAddressImpl::IPAddressImpl() {
}

IPAddressImpl::~IPAddressImpl() {
}

//*********************************************************************
//IPv4AddressImpl
//*********************************************************************

IPv4AddressImpl::IPv4AddressImpl() {
	std::memset(&addr_, 0, sizeof(addr_));
}

IPv4AddressImpl::IPv4AddressImpl(const struct in_addr * addr) {
	std::memcpy(&addr_, addr, sizeof(addr_));
}

IPv4AddressImpl::IPv4AddressImpl(const IPv4AddressImpl & rhs) {
	std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
}

IPv4AddressImpl & IPv4AddressImpl::operator=(const IPv4AddressImpl & rhs) {
	if (this != &rhs) {
		std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
	}
	return *this;
}

std::string IPv4AddressImpl::ToString() const {
	i8 buf[16];
	std::string result;
	if (0 == uv_inet_ntop(AF_INET, &addr_, buf, sizeof(buf))) {
		result = buf;
	}
	return result;
}

bool IPv4AddressImpl::operator==(const IPv4AddressImpl & rhs) const {
	return 0 == std::memcmp(&addr_, &rhs.addr_, sizeof(addr_));
}

bool IPv4AddressImpl::operator!=(const IPv4AddressImpl & rhs) const {
	return !(*this == rhs);
}

IPv4AddressImpl IPv4AddressImpl::Parse(const std::string & ip) {
	if (ip.empty()) {
		return IPv4AddressImpl();
	}

	struct sockaddr_in si;
	if (0 != uv_ip4_addr(ip.c_str(), 0, &si)) {
		return IPv4AddressImpl();
	} else {
		return IPv4AddressImpl(&si.sin_addr);
	}
}

//*********************************************************************
//IPv6AddressImpl
//*********************************************************************

IPv6AddressImpl::IPv6AddressImpl() : scope_(0) {
	std::memset(&addr_, 0, sizeof(addr_));
}

IPv6AddressImpl::IPv6AddressImpl(const struct in6_addr * addr, u32 scope) : scope_(scope) {
	std::memcpy(&addr_, addr, sizeof(addr_));
}

IPv6AddressImpl::IPv6AddressImpl(const IPv6AddressImpl & rhs) : scope_(rhs.scope_) {
	std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
}

IPv6AddressImpl & IPv6AddressImpl::operator=(const IPv6AddressImpl & rhs) {
	if (this != &rhs) {
		scope_ = rhs.scope_;
		std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
	}
	return *this;
}

std::string IPv6AddressImpl::ToString() const {
	i8 buf[46];
	std::string result;
	if (0 == uv_inet_ntop(AF_INET6, &addr_, buf, sizeof(buf))) {
		result = buf;
	}
	return result;
}

bool IPv6AddressImpl::operator==(const IPv6AddressImpl & rhs) const {
	return scope_ == rhs.scope_ && 0 == std::memcmp(&addr_, &rhs.addr_, sizeof(addr_));
}

bool IPv6AddressImpl::operator!=(const IPv6AddressImpl & rhs) const {
	return !(*this == rhs);
}

IPv6AddressImpl IPv6AddressImpl::Parse(const std::string & ip) {
	if (ip.empty()) {
		return IPv6AddressImpl();
	}

	struct sockaddr_in6 si;
	if (0 != uv_ip6_addr(ip.c_str(), 0, &si)) {
		return IPv6AddressImpl();
	} else {
		return IPv6AddressImpl(&si.sin6_addr, si.sin6_scope_id);
	}
}

}