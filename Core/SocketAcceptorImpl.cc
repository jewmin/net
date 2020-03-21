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

#include "Core/SocketAcceptorImpl.h"
#include "Core/SocketWrapper.h"

namespace Net {

SocketAcceptorImpl::SocketAcceptorImpl(EventReactor * reactor, SocketServer * server, i32 max_out_buffer_size, i32 max_in_buffer_size)
	: SocketAcceptor(reactor), server_(server), max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {
}

SocketAcceptorImpl::~SocketAcceptorImpl() {
}

void SocketAcceptorImpl::MakeConnection(SocketConnection * & connection) {
	SocketWrapper * wrapper = new SocketWrapper(server_, max_out_buffer_size_, max_in_buffer_size_);
	connection = wrapper->GetConnection();
}

void SocketAcceptorImpl::OnAccepted(SocketConnection * connection) {
}

}