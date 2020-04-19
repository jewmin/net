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

#include "Core/Client.h"

namespace Net {

Client::Client(const std::string & name, EventReactor * reactor, SocketConnector * connector, i32 max_out_buffer_size, i32 max_in_buffer_size, u32 object_max_count)
	: ConnectionMgr(name, object_max_count), reactor_(reactor), connector_(connector)
	, max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {
	if (connector_) {
		connector_->Duplicate();
	}
}

Client::~Client() {
	if (connector_) {
		connector_->Release();
	}
}

bool Client::Connect(const std::string & address, i32 port) {
	if (!connector_) {
		connector_ = new SocketConnector(reactor_);
	}
	Connection * connection = new Connection(this, max_out_buffer_size_, max_in_buffer_size_);
	return connector_->Connect(connection, SocketAddress(IPAddress(address), static_cast<u16>(port)));
}

}