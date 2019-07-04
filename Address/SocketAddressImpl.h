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

#ifndef Net_SocketAddressImpl_INCLUDE
#define Net_SocketAddressImpl_INCLUDE

#include "IPAddress.h"

namespace Net {
	class SocketAddressImpl {
	public:
		SocketAddressImpl(const SocketAddressImpl &) = delete;
		SocketAddressImpl & operator=(const SocketAddressImpl &) = delete;
		virtual ~SocketAddressImpl();

		virtual IPAddress Host() const = 0;
		virtual u16 Port() const = 0;
		virtual socklen_t Length() const = 0;
		virtual const struct sockaddr * Addr() const = 0;
		virtual int AF() const = 0;
		virtual AddressFamily::Family Family() const = 0;
		virtual std::string ToString() const = 0;

	protected:
		SocketAddressImpl();
	};

	class IPv4SocketAddressImpl : public SocketAddressImpl {
	public:
		IPv4SocketAddressImpl();
		explicit IPv4SocketAddressImpl(const struct sockaddr_in * addr);
		IPv4SocketAddressImpl(const void * addr, u16 port);

		virtual IPAddress Host() const;
		virtual u16 Port() const;
		virtual socklen_t Length() const;
		virtual const struct sockaddr * Addr() const;
		virtual int AF() const;
		virtual AddressFamily::Family Family() const;
		virtual std::string ToString() const;
		
	private:
		struct sockaddr_in addr_;
	};

	class IPv6SocketAddressImpl : public SocketAddressImpl {
	public:
		IPv6SocketAddressImpl();
		explicit IPv6SocketAddressImpl(const struct sockaddr_in6 * addr);
		IPv6SocketAddressImpl(const void * addr, u16 port, u32 scope = 0);

		virtual IPAddress Host() const;
		virtual u16 Port() const;
		virtual socklen_t Length() const;
		virtual const struct sockaddr * Addr() const;
		virtual int AF() const;
		virtual AddressFamily::Family Family() const;
		virtual std::string ToString() const;

	private:
		struct sockaddr_in6 addr_;

	};
}

//*********************************************************************
//IPv4SocketAddressImpl
//*********************************************************************

inline Net::IPAddress Net::IPv4SocketAddressImpl::Host() const {
	return IPAddress(&addr_.sin_addr, sizeof(addr_.sin_addr));
}

inline u16 Net::IPv4SocketAddressImpl::Port() const {
	return addr_.sin_port;
}

inline socklen_t Net::IPv4SocketAddressImpl::Length() const {
	return sizeof(addr_);
}

inline const struct sockaddr * Net::IPv4SocketAddressImpl::Addr() const {
	return reinterpret_cast<const struct sockaddr *>(&addr_);
}

inline int Net::IPv4SocketAddressImpl::AF() const {
	return addr_.sin_family;
}

inline Net::AddressFamily::Family Net::IPv4SocketAddressImpl::Family() const {
	return AddressFamily::IPv4;
}

//*********************************************************************
//IPv4SocketAddressImpl
//*********************************************************************

inline Net::IPAddress Net::IPv6SocketAddressImpl::Host() const {
	return IPAddress(&addr_.sin6_addr, sizeof(addr_.sin6_addr), addr_.sin6_scope_id);
}

inline u16 Net::IPv6SocketAddressImpl::Port() const {
	return addr_.sin6_port;
}

inline socklen_t Net::IPv6SocketAddressImpl::Length() const {
	return sizeof(addr_);
}

inline const struct sockaddr * Net::IPv6SocketAddressImpl::Addr() const {
	return reinterpret_cast<const struct sockaddr *>(&addr_);
}

inline int Net::IPv6SocketAddressImpl::AF() const {
	return addr_.sin6_family;
}

inline Net::AddressFamily::Family Net::IPv6SocketAddressImpl::Family() const {
	return AddressFamily::IPv6;
}

#endif