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

#include "Sockets/StreamSocket.h"
#include "Sockets/StreamSocketImpl.h"
#include "NetworkException.h"

namespace Net {

StreamSocket::StreamSocket() : Socket(new StreamSocketImpl()) {
}

StreamSocket::StreamSocket(SocketImpl * impl) : Socket(impl) {
	if (!dynamic_cast<StreamSocketImpl *>(Impl())) {
		throw NetworkException("socket impl != StreamSocketImpl");
	}
}

StreamSocket::StreamSocket(const Socket & other) : Socket(other) {
	if (!dynamic_cast<StreamSocketImpl *>(Impl())) {
		throw NetworkException("socket impl != StreamSocketImpl");
	}
}

StreamSocket & StreamSocket::operator=(const Socket & other) {
	if (dynamic_cast<StreamSocketImpl *>(other.Impl())) {
		Socket::operator=(other);
	} else {
		throw NetworkException("socket impl != StreamSocketImpl");
	}
	return *this;
}

StreamSocket::~StreamSocket() {
}

}