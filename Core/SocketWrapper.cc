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

#include "SocketWrapper.h"
#include "SocketWrapperMgr.h"
#include "SocketConnectionImpl.h"

Net::SocketWrapper::SocketWrapper(SocketWrapperMgr * mgr, int maxOutBufferSize, int maxInBufferSize) : mgr_(mgr), id_(0), register_mgr_(false) {
	if (!mgr_) {
		throw std::invalid_argument("mgr is nullptr");
	}
	connection_ = new SocketConnectionImpl(this, maxOutBufferSize, maxInBufferSize);
}

Net::SocketWrapper::~SocketWrapper() {
	if (connection_) {
		connection_->Destroy();
		connection_ = nullptr;
	}
}

void Net::SocketWrapper::Shutdown() {
	connection_->Shutdown(false);
}

void Net::SocketWrapper::ShutdownNow() {
	connection_->Shutdown(true);
}

int Net::SocketWrapper::Write(const char * data, int len) {
	return connection_->Write(data, len);
}

int Net::SocketWrapper::Read(char * data, int len) {
	return connection_->Read(data, len);
}

char * Net::SocketWrapper::GetRecvData() const {
	return connection_->GetRecvData();
}

int Net::SocketWrapper::GetRecvDataSize() const {
	return connection_->GetRecvDataSize();
}

void Net::SocketWrapper::PopRecvData(int size) {
	connection_->PopRecvData(size);
}

void Net::SocketWrapper::SetMaxOutBufferSize(int size) {
	connection_->SetMaxOutBufferSize(size);
}

void Net::SocketWrapper::SetMaxInBufferSize(int size) {
	connection_->SetMaxInBufferSize(size);
}

int Net::SocketWrapper::GetMaxOutBufferSize() const {
	return connection_->GetMaxOutBufferSize();
}

int Net::SocketWrapper::GetMaxInBufferSize() const {
	return connection_->GetMaxInBufferSize();
}

int Net::SocketWrapper::GetOutBufferUsedSize() {
	return connection_->GetOutBufferUsedSize();
}

Net::SocketConnection * Net::SocketWrapper::GetConnection() const {
	return connection_;
}

void Net::SocketWrapper::OnConnected() {
	mgr_->Register(this);
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnConnected(this);
	}
}

void Net::SocketWrapper::OnConnectFailed(int reason) {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnConnectFailed(this, reason);
	}
	mgr_->UnRegister(this);
}

void Net::SocketWrapper::OnDisconnected(bool isRemote) {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnDisconnected(this, isRemote);
	}
	mgr_->UnRegister(this);
}

void Net::SocketWrapper::OnNewDataReceived() {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnNewDataReceived(this);
	}
}

void Net::SocketWrapper::OnSomeDataSent() {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnSomeDataSent(this);
	}
}

void Net::SocketWrapper::OnError(int reason) {
	if (UV_EOF == reason) {
		LogInfo("连接[%u] OnError(%s): 收到对端EOF, 正常断开", id_, uv_err_name(reason));
	} else if (UV_ECANCELED != reason) {
		LogWarn("连接[%u] OnError(%s): %s", id_, uv_err_name(reason), uv_strerror(reason));
	}
}