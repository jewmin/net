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

#include "Interface/Client.h"

namespace Net {

Client::Client(const std::string & name, EventReactor * reactor, SocketConnector * connector, i32 max_out_buffer_size, i32 max_in_buffer_size)
	: ConnectionMgr(name), reactor_(reactor), local_connector_(nullptr), connector_ref_(nullptr)
	, max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {
	if (connector) {
		connector_ref_ = connector->WeakRef();
	}
}

Client::~Client() {
	if (connector_ref_) {
		connector_ref_->Release();
	}
	if (local_connector_) {
		delete local_connector_;
	}
}

i64 Client::Connect(const std::string & address, i32 port) {
	if (!connector_ref_) {
		local_connector_ = new SocketConnector(reactor_);
		connector_ref_ = local_connector_->WeakRef();
	}

	Connection * connection = new Connection(this, max_out_buffer_size_, max_in_buffer_size_);
	Register(connection);

	SocketConnector * connector = dynamic_cast<SocketConnector *>(connector_ref_->Get());
	if (connector && connector->Connect(connection, SocketAddress(address, static_cast<u16>(port)))) {
		return connection->GetConnectionId();
	} else {
		return -1;
	}
}

}