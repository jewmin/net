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

#ifndef Net_SocketConnector_INCLUDED
#define Net_SocketConnector_INCLUDED

#include "Sockets/StreamSocket.h"
#include "SocketReactor.h"

namespace Net {
	template<class ServiceHandler>
	class SocketConnector {
	public:
		explicit SocketConnector(StreamSocket & socket, SocketAddress & address);

	protected:
		virtual ServiceHandler * CreateServiceHandler(StreamSocket & socket);
		SocketReactor * GetReactor();
		Socket & GetSocket();

	private:
		SocketConnector() = delete;
		SocketConnector(const SocketConnector &) = delete;
		SocketConnector & operator=(const SocketConnector &) = delete;

		StreamSocket socket_;
		SocketReactor * reactor_;
	};
}

template<class ServiceHandler>
inline Net::SocketReactor * Net::SocketConnector<ServiceHandler>::GetReactor() {
	return reactor_;
}

template<class ServiceHandler>
inline Net::Socket & Net::SocketConnector<ServiceHandler>::GetSocket() {
	return socket_;
}

#endif