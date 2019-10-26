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

#include "SocketWrapperMgr.h"
#include "Logger.h"

Net::SocketWrapperMgr::SocketWrapperMgr(const std::string & name) : event_(nullptr), name_(name) {
	socket_list_ = new Foundation::ObjectMgr<SocketWrapper>(64, 1024);
	need_to_shutdown_list_ = new std::list<u32>();
	need_to_delete_list_ = new std::list<u32>();
}

Net::SocketWrapperMgr::~SocketWrapperMgr() {
	ShutDownAllSocketWrappers();
	if (socket_list_) {
		delete socket_list_;
		socket_list_ = nullptr;
	}
	if (need_to_shutdown_list_) {
		delete need_to_shutdown_list_;
		need_to_shutdown_list_ = nullptr;
	}
	if (need_to_delete_list_) {
		delete need_to_delete_list_;
		need_to_delete_list_ = nullptr;
	}
}

u32 Net::SocketWrapperMgr::Register(SocketWrapper * wrapper) {
	u32 id = wrapper->GetId();
	if (!wrapper->GetIsRegister2Mgr()) {
		if (id > 0) {
			socket_list_->AddNewObj(id, wrapper);
		} else {
			id = socket_list_->AddNewObj(wrapper);
			wrapper->SetId(id);
		}
		wrapper->SetIsRegister2Mgr(true);	
	}
	return id;
}

void Net::SocketWrapperMgr::UnRegister(SocketWrapper * wrapper) {
	if (wrapper->GetIsRegister2Mgr()) {
		wrapper->SetIsRegister2Mgr(false);
		// 这里不能直接删除，因为回调处理中可能是用到对象
		// socket_list_->DeleteObj(wrapper->GetId());
		need_to_delete_list_->push_back(wrapper->GetId());
	}
}

Net::SocketWrapper * Net::SocketWrapperMgr::GetSocketWrapper(u32 id) {
	return socket_list_->GetObj(id);
}

void Net::SocketWrapperMgr::ShutDownAllSocketWrappers() {
	socket_list_->EnumEachObj(ShutDownOneSocketWrapper, nullptr);
}

void Net::SocketWrapperMgr::ShutDownOneSocketWrapper(u32 id) {
	SocketWrapper * wrapper = GetSocketWrapper(id);
	if (wrapper) {
		wrapper->Shutdown();
	}
}

void Net::SocketWrapperMgr::InsertToNeedToShutdownList(u32 id) {
	need_to_shutdown_list_->push_back(id);
}

u32 Net::SocketWrapperMgr::GetSocketWrapperCount() const {
	return socket_list_->GetObjCount();
}

void Net::SocketWrapperMgr::Update() {
	CleanDeathConnection();
}

void Net::SocketWrapperMgr::CleanDeathConnection() {
	while (!need_to_shutdown_list_->empty()) {
		ShutDownOneSocketWrapper(need_to_shutdown_list_->front());
		need_to_shutdown_list_->pop_front();
	}
	while (!need_to_delete_list_->empty()) {
		socket_list_->DeleteObj(need_to_delete_list_->front());
		need_to_delete_list_->pop_front();
	}
}

void Net::SocketWrapperMgr::ShutDownOneSocketWrapper(void * wrapper, void * ud) {
	static_cast<SocketWrapper *>(wrapper)->Shutdown();
}