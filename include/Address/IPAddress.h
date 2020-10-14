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

#ifndef Net_Address_IPAddress_INCLUDED
#define Net_Address_IPAddress_INCLUDED

#include "Common.h"
#include "CObject.h"
#include "SDString.h"
#include "Address/IPAddressImpl.h"
#include "uv.h"

namespace Net {

class COMMON_EXTERN IPAddress : public Common::CObject {
public:
	IPAddress();
	explicit IPAddress(const i8 * ip);
	IPAddress(const void * addr, socklen_t length, u32 scope = 0);
	IPAddress(const IPAddress & other);
	IPAddress & operator=(const IPAddress & other);
	virtual ~IPAddress();

	Common::SDString ToString() const;
	socklen_t Length() const;
	const void * Addr() const;
	AddressFamily::eFamily Family() const;
	i32 AF() const;
	u32 Scope() const;
	bool operator==(const IPAddress & other) const;
	bool operator!=(const IPAddress & other) const;

	static IPAddress Parse(const i8 * ip);
	static bool TryParse(const i8 * ip, IPAddress & result);

	static const AddressFamily::eFamily IPv4 = AddressFamily::IPv4;
	static const AddressFamily::eFamily IPv6 = AddressFamily::IPv6;

private:
	IPAddressImpl * Impl() const;
	void NewIPv4();
	void NewIPv4(const void * addr);
	void NewIPv6();
	void NewIPv6(const void * addr, u32 scope);
	void Destroy();
	i8 * Storage();

private:
	i8 memory_[sizeof(IPv6AddressImpl)];
};

inline Common::SDString IPAddress::ToString() const {
	return Impl()->ToString();
}

inline socklen_t IPAddress::Length() const {
	return Impl()->Length();
}

inline const void * IPAddress::Addr() const {
	return Impl()->Addr();
}

inline AddressFamily::eFamily IPAddress::Family() const {
	return Impl()->Family();
}

inline i32 IPAddress::AF() const {
	return Impl()->AF();
}

inline u32 IPAddress::Scope() const {
	return Impl()->Scope();
}

inline IPAddressImpl * IPAddress::Impl() const {
	return reinterpret_cast<IPAddressImpl *>(const_cast<i8 *>(memory_));
}

inline i8 * IPAddress::Storage() {
	return memory_;
}

}

#endif