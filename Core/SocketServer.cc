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

#include "SocketServer.h"
#include "SocketAcceptorImpl.h"

Net::SocketServer::SocketServer(const std::string & name, EventReactor * reactor, int maxOutBufferSize, int maxInBufferSize)
	: SocketWrapperMgr(name), reactor_(reactor), accpetor_(nullptr)
	, max_out_buffer_size_(maxOutBufferSize), max_in_buffer_size_(maxInBufferSize), listen_(false) {
}

Net::SocketServer::~SocketServer() {
	Terminate();
}

bool Net::SocketServer::Listen(const std::string & address, int port, int backlog, bool ipV6Only) {
	if (listen_) {
		return false;
	}
	if (!accpetor_) {
		accpetor_ = new SocketAcceptorImpl(reactor_, this, max_out_buffer_size_, max_in_buffer_size_);
	}
	if (accpetor_->Open(SocketAddress(address, static_cast<u16>(port)), backlog, ipV6Only)) {
		listen_ = true;
		return true;
	} else {
		accpetor_->Destroy();
		accpetor_ = nullptr;
		return false;
	}
}

bool Net::SocketServer::Terminate() {
	if (!listen_) {
		return false;
	}
	listen_ = false;
	if (accpetor_) {
		accpetor_->Destroy();
		accpetor_ = nullptr;
	}
	return true;
}