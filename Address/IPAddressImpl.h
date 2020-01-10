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

#ifndef Net_IPAddressImpl_INCLUDED
#define Net_IPAddressImpl_INCLUDED

#include "Net.h"
#include "AddressFamily.h"

namespace Net {

template<class S>
S Trim(const S & str) {
	i32 first = 0;
	i32 last = str.size() - 1;

	while (first <= last && 0x20 == str[first]) ++first;
	while (last >= first && 0x20 == str[last]) --last;

	return S(str, first, last - first + 1);
}

class IPAddressImpl {
public:
	virtual ~IPAddressImpl();

	virtual std::string ToString() const = 0;
	virtual socklen_t Length() const = 0;
	virtual const void * Addr() const = 0;
	virtual AddressFamily::Family Family() const = 0;
	virtual i32 AF() const = 0;
	virtual u32 Scope() const = 0;

protected:
	IPAddressImpl();

private:
	IPAddressImpl(const IPAddressImpl &) = delete;
	IPAddressImpl & operator=(const IPAddressImpl &) = delete;
};

class IPv4AddressImpl : public IPAddressImpl {
public:
	IPv4AddressImpl();
	explicit IPv4AddressImpl(const in_addr * addr);
	IPv4AddressImpl(const IPv4AddressImpl & rhs);
	IPv4AddressImpl & operator=(const IPv4AddressImpl & rhs);
	
	virtual std::string ToString() const;
	virtual socklen_t Length() const;
	virtual const void * Addr() const;
	virtual AddressFamily::Family Family() const;
	virtual i32 AF() const;
	virtual u32 Scope() const;
	bool operator==(const IPv4AddressImpl & rhs) const;
	bool operator!=(const IPv4AddressImpl & rhs) const;

	static IPv4AddressImpl Parse(const std::string & ip);

private:
	struct in_addr addr_;
};

class IPv6AddressImpl : public IPAddressImpl {
public:
	IPv6AddressImpl();
	explicit IPv6AddressImpl(const in6_addr * addr, u32 scope = 0);
	IPv6AddressImpl(const IPv6AddressImpl & rhs);
	IPv6AddressImpl & operator=(const IPv6AddressImpl & rhs);

	virtual std::string ToString() const;
	virtual socklen_t Length() const;
	virtual const void * Addr() const;
	virtual AddressFamily::Family Family() const;
	virtual i32 AF() const;
	virtual u32 Scope() const;
	bool operator==(const IPv6AddressImpl & rhs) const;
	bool operator!=(const IPv6AddressImpl & rhs) const;

	static IPv6AddressImpl Parse(const std::string & ip);

private:
	struct in6_addr addr_;
	u32 scope_;
};

//*********************************************************************
//IPv4AddressImpl
//*********************************************************************

inline socklen_t IPv4AddressImpl::Length() const {
	return sizeof(addr_);
}

inline const void * IPv4AddressImpl::Addr() const {
	return &addr_;
}

inline AddressFamily::Family IPv4AddressImpl::Family() const {
	return AddressFamily::IPv4;
}

inline i32 IPv4AddressImpl::AF() const {
	return AF_INET;
}

inline u32 IPv4AddressImpl::Scope() const {
	return 0;
}

//*********************************************************************
//IPv6AddressImpl
//*********************************************************************

inline socklen_t IPv6AddressImpl::Length() const {
	return sizeof(addr_);
}

inline const void * IPv6AddressImpl::Addr() const {
	return &addr_;
}

inline AddressFamily::Family IPv6AddressImpl::Family() const {
	return AddressFamily::IPv6;
}

inline i32 IPv6AddressImpl::AF() const {
	return AF_INET6;
}

inline u32 IPv6AddressImpl::Scope() const {
	return scope_;
}

}

#endif