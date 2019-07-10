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

Net::IPAddressImpl::IPAddressImpl() {
}

Net::IPAddressImpl::~IPAddressImpl() {
}

//*********************************************************************
//IPv4AddressImpl
//*********************************************************************

Net::IPv4AddressImpl::IPv4AddressImpl() {
	std::memset(&addr_, 0, sizeof(addr_));
}

Net::IPv4AddressImpl::IPv4AddressImpl(const void * addr) {
	std::memcpy(&addr_, addr, sizeof(addr_));
}

Net::IPv4AddressImpl::IPv4AddressImpl(const IPv4AddressImpl & rhs) {
	std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
}

Net::IPv4AddressImpl & Net::IPv4AddressImpl::operator=(const IPv4AddressImpl & rhs) {
	if (this != &rhs) {
		std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
	}
	
	return *this;
}

Net::IPAddressImpl * Net::IPv4AddressImpl::Clone() const {
	return new IPv4AddressImpl(*this);
}

std::string Net::IPv4AddressImpl::ToString() const {
	char buf[16];
	std::string result;
	int err = uv_inet_ntop(AF_INET, &addr_, buf, sizeof(buf));
	if (0 == err) {
		result = buf;
	}

	return result;
}

bool Net::IPv4AddressImpl::operator==(const IPv4AddressImpl & addr) const {
	return 0 == std::memcmp(&addr_, &addr.addr_, sizeof(addr_));
}

bool Net::IPv4AddressImpl::operator!=(const IPv4AddressImpl & addr) const {
	return !(*this == addr);
}

Net::IPv4AddressImpl Net::IPv4AddressImpl::Parse(const std::string & addr) {
	if (addr.empty()) {
		return IPv4AddressImpl();
	}

	struct sockaddr_in si;
	int err = uv_ip4_addr(addr.c_str(), 0, &si);
	if (0 != err) {
		return IPv4AddressImpl();
	} else {
		return IPv4AddressImpl(&si.sin_addr);
	}
}

//*********************************************************************
//IPv6AddressImpl
//*********************************************************************

Net::IPv6AddressImpl::IPv6AddressImpl() : scope_(0) {
	std::memset(&addr_, 0, sizeof(addr_));
}

Net::IPv6AddressImpl::IPv6AddressImpl(const void * addr, u32 scope) : scope_(scope) {
	std::memcpy(&addr_, addr, sizeof(addr_));
}

Net::IPv6AddressImpl::IPv6AddressImpl(const IPv6AddressImpl & rhs) : scope_(rhs.scope_) {
	std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
}

Net::IPv6AddressImpl & Net::IPv6AddressImpl::operator=(const IPv6AddressImpl & rhs) {
	if (this != &rhs) {
		scope_ = rhs.scope_;
		std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
	}
	
	return *this;
}

Net::IPAddressImpl * Net::IPv6AddressImpl::Clone() const {
	return new IPv6AddressImpl(*this);
}

std::string Net::IPv6AddressImpl::ToString() const {
	char buf[46];
	std::string result;
	int err = uv_inet_ntop(AF_INET6, &addr_, buf, sizeof(buf));
	if (0 == err) {
		result = buf;
	}

	return result;
}

bool Net::IPv6AddressImpl::operator==(const IPv6AddressImpl & addr) const {
	return scope_ == addr.scope_ && 0 == std::memcmp(&addr_, &addr.addr_, sizeof(addr_));
}

bool Net::IPv6AddressImpl::operator!=(const IPv6AddressImpl & addr) const {
	return !(*this == addr);
}

Net::IPv6AddressImpl Net::IPv6AddressImpl::Parse(const std::string & addr) {
	if (addr.empty()) {
		return IPv6AddressImpl();
	}
	
	struct sockaddr_in6 si;
	int err = uv_ip6_addr(addr.c_str(), 0, &si);
	if (0 != err) {
		return IPv6AddressImpl();
	} else {
		return IPv6AddressImpl(&si.sin6_addr, si.sin6_scope_id);
	}
}