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

#include "Core/SocketWrapper.h"
#include "Core/SocketWrapperMgr.h"
#include "Core/SocketConnectionImpl.h"
#include "Common/Logger.h"

namespace Net {

SocketWrapper::SocketWrapper(SocketWrapperMgr * mgr, int maxOutBufferSize, int maxInBufferSize) : id_(0), is_raw_recv_(false), register_mgr_(false), mgr_(mgr) {
	if (!mgr_) {
		Log(kCrash, __FILE__, __LINE__, "mgr is nullptr");
	}
	connection_ = new SocketConnectionImpl(this, maxOutBufferSize, maxInBufferSize);
}

SocketWrapper::~SocketWrapper() {
	if (connection_) {
		connection_->Destroy();
		connection_ = nullptr;
	}
}

void SocketWrapper::Shutdown() {
	connection_->Shutdown(false);
}

void SocketWrapper::ShutdownNow() {
	connection_->Shutdown(true);
}

void SocketWrapper::NeedToShutdown() {
	mgr_->InsertToNeedToShutdownList(id_);
}

int SocketWrapper::Write(const char * data, int len) {
	int status = 0;
	if (len > 0) {
		status = connection_->Write(data, len);
		if (status < 0) {
			connection_->Error(status);
		}
	}
	return status;
}

int SocketWrapper::Read(char * data, int len) {
	return connection_->Read(data, len);
}

char * SocketWrapper::GetRecvData() const {
	return connection_->GetRecvData();
}

int SocketWrapper::GetRecvDataSize() const {
	return connection_->GetRecvDataSize();
}

void SocketWrapper::PopRecvData(int size) {
	connection_->PopRecvData(size);
}

void SocketWrapper::SetMaxOutBufferSize(int size) {
	connection_->SetMaxOutBufferSize(size);
}

void SocketWrapper::SetMaxInBufferSize(int size) {
	connection_->SetMaxInBufferSize(size);
}

int SocketWrapper::GetMaxOutBufferSize() const {
	return connection_->GetMaxOutBufferSize();
}

int SocketWrapper::GetMaxInBufferSize() const {
	return connection_->GetMaxInBufferSize();
}

int SocketWrapper::GetOutBufferUsedSize() {
	return connection_->GetOutBufferUsedSize();
}

SocketConnection * SocketWrapper::GetConnection() const {
	return connection_;
}

void SocketWrapper::OnConnected() {
	mgr_->Register(this);
	IEvent * event = mgr_->GetEvent();
	if (event) {
		if (event->OnConnected(this) == 1) {
			NeedToShutdown();
		}
	}
}

void SocketWrapper::OnConnectFailed(int reason) {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnConnectFailed(this, reason);
	}
	mgr_->UnRegister(this);
}

void SocketWrapper::OnDisconnected(bool isRemote) {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		event->OnDisconnected(this, isRemote);
	}
	mgr_->UnRegister(this);
}

void SocketWrapper::OnNewDataReceived() {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		if (event->OnNewDataReceived(this) == 1) {
			NeedToShutdown();
		}
	}
}

void SocketWrapper::OnSomeDataSent() {
	IEvent * event = mgr_->GetEvent();
	if (event) {
		if (event->OnSomeDataSent(this) == 1) {
			NeedToShutdown();
		}
	}
}

void SocketWrapper::OnError(int reason) {
	if (UV_EOF == reason) {
		Log(kLog, __FILE__, __LINE__, "连接[", id_, uv_err_name(reason), "]: 收到对端EOF, 正常断开");
	} else if (UV_ECANCELED != reason) {
		Log(kLog, __FILE__, __LINE__, "连接", id_, uv_err_name(reason), uv_strerror(reason));
	}
}

}