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

#include "Reactor/EventReactor.h"

namespace Net {

EventReactor::EventReactor() : loop_(static_cast<uv_loop_t *>(jc_malloc(sizeof(uv_loop_t)))) {
	Logger::Category::GetCategory("EventReactor")->Info("<libuv> %s", uv_version_string());
	uv_loop_init(loop_);
	loop_->data = this;
}

EventReactor::~EventReactor() {
	ClearEventHandlers();
	while (Poll()) {
		Poll(UV_RUN_ONCE);
	}
	uv_loop_close(loop_);
	jc_free(loop_);
}

bool EventReactor::AddEventHandler(EventHandler * handler) {
	bool success = handler->RegisterToReactor();
	if (success) {
		handler->Duplicate();
		handlers_.PushBack(handler);
	}
	return success;
}

bool EventReactor::RemoveEventHandler(EventHandler * handler) {
	bool success = handler->UnRegisterFromReactor();
	if (success) {
		handler->RemoveFromList();
		handler->Release();
	}
	return success;
}

void EventReactor::ClearEventHandlers() {
	EventHandler * handler = handlers_.Front();
	while (handler) {
		RemoveEventHandler(handler);
		handler = handlers_.Front();
	}
}

}