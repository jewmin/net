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

#include "SocketAddressImpl.h"

Net::SocketAddressImpl::SocketAddressImpl() {
}

Net::SocketAddressImpl::~SocketAddressImpl() {
}

//*********************************************************************
//IPv4SocketAddressImpl
//*********************************************************************

Net::IPv4SocketAddressImpl::IPv4SocketAddressImpl() {
	std::memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
}

Net::IPv4SocketAddressImpl::IPv4SocketAddressImpl(const struct sockaddr_in * addr) {
	std::memcpy(&addr_, addr, sizeof(addr_));
}

Net::IPv4SocketAddressImpl::IPv4SocketAddressImpl(const void * addr, u16 port) {
	std::memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	std::memcpy(&addr_.sin_addr, addr, sizeof(addr_.sin_addr));
	addr_.sin_port = htons(port);
}

std::string Net::IPv4SocketAddressImpl::ToString() const {
	std::stringstream result;
	result << Host().ToString() << ":" << Port();
	return result.str();
}

//*********************************************************************
//IPv6SocketAddressImpl
//*********************************************************************

Net::IPv6SocketAddressImpl::IPv6SocketAddressImpl() {
	std::memset(&addr_, 0, sizeof(addr_));
	addr_.sin6_family = AF_INET6;
}

Net::IPv6SocketAddressImpl::IPv6SocketAddressImpl(const struct sockaddr_in6 * addr) {
	std::memcpy(&addr_, addr, sizeof(addr_));
}

Net::IPv6SocketAddressImpl::IPv6SocketAddressImpl(const void * addr, u16 port, u32 scope) {
	std::memset(&addr_, 0, sizeof(addr_));
	addr_.sin6_family = AF_INET6;
	std::memcpy(&addr_.sin6_addr, addr, sizeof(addr_.sin6_addr));
	addr_.sin6_port = htons(port);
	addr_.sin6_scope_id = scope;
}

std::string Net::IPv6SocketAddressImpl::ToString() const {
	std::stringstream result;
	result << "[" << Host().ToString() << "]:" << Port();
	return result.str();
}
