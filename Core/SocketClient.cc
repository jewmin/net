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

#include "Logger.h"
#include "SocketClient.h"

Net::SocketClient::SocketClient(const std::string & name, EventReactor * reactor, SocketConnector * connector, int maxOutBufferSize, int maxInBufferSize)
	: SocketWrapperMgr(name), reactor_(reactor), connector_(connector), max_out_buffer_size_(maxOutBufferSize), max_in_buffer_size_(maxInBufferSize) {
	if (connector_) {
		connector_->Duplicate();
	}
}

Net::SocketClient::~SocketClient() {
	Terminate();
}

bool Net::SocketClient::Connect(const std::string & address, int port, u32 & id) {
	id = 0;
	if (!connector_) {
		connector_ = new SocketConnector(reactor_);
	}
	SocketWrapper * wrapper = new SocketWrapper(this, max_out_buffer_size_, max_in_buffer_size_);
	id = Register(wrapper);
	SocketAddress sa(address, static_cast<u16>(port));
	int status = connector_->Connect(wrapper->GetConnection(), sa);
	if (status < 0) {
		Foundation::LogWarn("%s: 建立连接[%s]失败", GetName(), sa.ToString());
		return false;
	}
	return true;
}

bool Net::SocketClient::Terminate() {
	if (connector_) {
		connector_->Destroy();
		connector_ = nullptr;
	}
	return true;
}