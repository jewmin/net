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

#ifndef Net_Address_IPAddressImpl_INCLUDED
#define Net_Address_IPAddressImpl_INCLUDED

#include "Common.h"
#include "CObject.h"
#include "SDString.h"
#include "Address/AddressFamily.h"
#include "uv.h"

namespace Net {

class COMMON_EXTERN IPAddressImpl : public Common::CObject {
public:
	virtual ~IPAddressImpl();

	virtual Common::SDString ToString() const = 0;
	virtual socklen_t Length() const = 0;
	virtual const void * Addr() const = 0;
	virtual AddressFamily::eFamily Family() const = 0;
	virtual i32 AF() const = 0;
	virtual u32 Scope() const = 0;

protected:
	IPAddressImpl();

private:
	IPAddressImpl(IPAddressImpl &&) = delete;
	IPAddressImpl(const IPAddressImpl &) = delete;
	IPAddressImpl & operator=(IPAddressImpl &&) = delete;
	IPAddressImpl & operator=(const IPAddressImpl &) = delete;
};

class COMMON_EXTERN IPv4AddressImpl : public IPAddressImpl {
public:
	IPv4AddressImpl();
	explicit IPv4AddressImpl(const void * addr);
	IPv4AddressImpl(const IPv4AddressImpl & other);
	IPv4AddressImpl & operator=(const IPv4AddressImpl & other);
	virtual ~IPv4AddressImpl();

	virtual Common::SDString ToString() const override;
	virtual socklen_t Length() const override;
	virtual const void * Addr() const override;
	virtual AddressFamily::eFamily Family() const override;
	virtual i32 AF() const override;
	virtual u32 Scope() const override;
	bool operator==(const IPv4AddressImpl & other) const;
	bool operator!=(const IPv4AddressImpl & other) const;

	static IPv4AddressImpl Parse(const i8 * ip);

private:
	struct in_addr addr_;
};

class COMMON_EXTERN IPv6AddressImpl : public IPAddressImpl {
public:
	IPv6AddressImpl();
	IPv6AddressImpl(const void * addr, u32 scope);
	IPv6AddressImpl(const IPv6AddressImpl & other);
	IPv6AddressImpl & operator=(const IPv6AddressImpl & other);
	virtual ~IPv6AddressImpl();

	virtual Common::SDString ToString() const override;
	virtual socklen_t Length() const override;
	virtual const void * Addr() const override;
	virtual AddressFamily::eFamily Family() const override;
	virtual i32 AF() const override;
	virtual u32 Scope() const override;
	bool operator==(const IPv6AddressImpl & other) const;
	bool operator!=(const IPv6AddressImpl & other) const;

	static IPv6AddressImpl Parse(const i8 * ip);

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

inline AddressFamily::eFamily IPv4AddressImpl::Family() const {
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

inline AddressFamily::eFamily IPv6AddressImpl::Family() const {
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