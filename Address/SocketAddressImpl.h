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

#ifndef Net_SocketAddressImpl_INCLUDED
#define Net_SocketAddressImpl_INCLUDED

#include "IPAddress.h"

namespace Net {

class SocketAddressImpl : public NetObject {
public:
	virtual ~SocketAddressImpl();

	virtual IPAddress Host() const = 0;
	virtual u16 Port() const = 0;
	virtual socklen_t Length() const = 0;
	virtual const struct sockaddr * Addr() const = 0;
	virtual i32 AF() const = 0;
	virtual AddressFamily::Family Family() const = 0;
	virtual std::string ToString() const = 0;

protected:
	SocketAddressImpl();

private:
	SocketAddressImpl(const SocketAddressImpl &) = delete;
	SocketAddressImpl & operator=(const SocketAddressImpl &) = delete;
};

class IPv4SocketAddressImpl : public SocketAddressImpl {
public:
	IPv4SocketAddressImpl();
	explicit IPv4SocketAddressImpl(const struct sockaddr_in * addr);
	virtual ~IPv4SocketAddressImpl();

	virtual IPAddress Host() const;
	virtual u16 Port() const;
	virtual socklen_t Length() const;
	virtual const struct sockaddr * Addr() const;
	virtual i32 AF() const;
	virtual AddressFamily::Family Family() const;
	virtual std::string ToString() const;
	
private:
	struct sockaddr_in addr_;
};

class IPv6SocketAddressImpl : public SocketAddressImpl {
public:
	IPv6SocketAddressImpl();
	explicit IPv6SocketAddressImpl(const struct sockaddr_in6 * addr);
	virtual ~IPv6SocketAddressImpl();

	virtual IPAddress Host() const;
	virtual u16 Port() const;
	virtual socklen_t Length() const;
	virtual const struct sockaddr * Addr() const;
	virtual i32 AF() const;
	virtual AddressFamily::Family Family() const;
	virtual std::string ToString() const;

private:
	struct sockaddr_in6 addr_;
};

//*********************************************************************
//IPv4SocketAddressImpl
//*********************************************************************

inline IPAddress IPv4SocketAddressImpl::Host() const {
	return IPAddress(&addr_.sin_addr, sizeof(addr_.sin_addr));
}

inline u16 IPv4SocketAddressImpl::Port() const {
	return ntohs(addr_.sin_port);
}

inline socklen_t IPv4SocketAddressImpl::Length() const {
	return sizeof(addr_);
}

inline const struct sockaddr * IPv4SocketAddressImpl::Addr() const {
	return reinterpret_cast<const struct sockaddr *>(&addr_);
}

inline i32 IPv4SocketAddressImpl::AF() const {
	return addr_.sin_family;
}

inline AddressFamily::Family IPv4SocketAddressImpl::Family() const {
	return AddressFamily::IPv4;
}

//*********************************************************************
//IPv4SocketAddressImpl
//*********************************************************************

inline IPAddress IPv6SocketAddressImpl::Host() const {
	return IPAddress(&addr_.sin6_addr, sizeof(addr_.sin6_addr), addr_.sin6_scope_id);
}

inline u16 IPv6SocketAddressImpl::Port() const {
	return ntohs(addr_.sin6_port);
}

inline socklen_t IPv6SocketAddressImpl::Length() const {
	return sizeof(addr_);
}

inline const struct sockaddr * IPv6SocketAddressImpl::Addr() const {
	return reinterpret_cast<const struct sockaddr *>(&addr_);
}

inline i32 IPv6SocketAddressImpl::AF() const {
	return addr_.sin6_family;
}

inline AddressFamily::Family IPv6SocketAddressImpl::Family() const {
	return AddressFamily::IPv6;
}

}

#endif