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

#ifndef Net_Address_SocketAddress_INCLUDED
#define Net_Address_SocketAddress_INCLUDED

#include "Common.h"
#include "CObject.h"
#include "SDString.h"
#include "Address/SocketAddressImpl.h"
#include "uv.h"

namespace Net {

class COMMON_EXTERN SocketAddress : public Common::CObject {
public:
	SocketAddress();
	explicit SocketAddress(u16 port);
	SocketAddress(const i8 * ip, u16 port);
	SocketAddress(const IPAddress & host, u16 port);
	SocketAddress(const struct sockaddr * addr, socklen_t length);
	SocketAddress(const SocketAddress & other);
	SocketAddress & operator=(const SocketAddress & other);
	virtual ~SocketAddress();

	IPAddress Host() const;
	u16 Port() const;
	socklen_t Length() const;
	const struct sockaddr * Addr() const;
	i32 AF() const;
	AddressFamily::eFamily Family() const;
	Common::SDString ToString() const;
	bool operator==(const SocketAddress & other) const;
	bool operator!=(const SocketAddress & other) const;

protected:
	void Init(const IPAddress & host, u16 port);

private:
	SocketAddressImpl * Impl() const;

	void NewIPv4();
	void NewIPv4(const struct sockaddr_in * addr);
	void NewIPv6(const struct sockaddr_in6 * addr);
	void Destroy();
	i8 * Storage();

private:
	i8 memory_[sizeof(IPv6SocketAddressImpl)];
};

inline IPAddress SocketAddress::Host() const {
	return Impl()->Host();
}

inline u16 SocketAddress::Port() const {
	return Impl()->Port();
}

inline socklen_t SocketAddress::Length() const {
	return Impl()->Length();
}

inline const sockaddr * SocketAddress::Addr() const {
	return Impl()->Addr();
}

inline i32 SocketAddress::AF() const {
	return Impl()->AF();
}

inline AddressFamily::eFamily SocketAddress::Family() const {
	return Impl()->Family();
}

inline Common::SDString SocketAddress::ToString() const {
	return Impl()->ToString();
}

inline SocketAddressImpl * SocketAddress::Impl() const {
	return reinterpret_cast<SocketAddressImpl *>(const_cast<i8 *>(memory_));
}

inline i8 * SocketAddress::Storage() {
	return memory_;
}

}

#endif