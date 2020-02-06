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
#include "EventReactor.h"
#include "EventHandler.h"

namespace Net {

EventReactor::EventReactor() {
	Log(kLog, __FILE__, __LINE__, "使用网络底层<libuv>");
	loop_ = static_cast<uv_loop_t *>(Allocator::Get()->Allocate(sizeof(uv_loop_t)));
	uv_loop_init(loop_);
}

EventReactor::~EventReactor() {
	while (!handlers_.empty()) {
		RemoveEventHandler(handlers_.front());
	}
	while (Dispatch()) {
		Dispatch(UV_RUN_ONCE);
	}
	uv_loop_close(loop_);
	Allocator::Get()->DeAllocate(loop_, sizeof(uv_loop_t));
}

bool EventReactor::AddEventHandler(EventHandler * handler) {
	bool success = handler->RegisterToReactor();
	if (success) {
		handlers_.push_back(handler);
	}
	return success;
}

bool EventReactor::RemoveEventHandler(EventHandler * handler) {
	bool success = handler->UnRegisterFromReactor();
	if (success) {
		handlers_.remove(handler);
	}
	return success;
}

}