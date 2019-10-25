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

#include "TimerImpl.h"

Foundation::TimerImpl::TimerImpl() : handle_(nullptr) {
}

Foundation::TimerImpl::~TimerImpl() {
	Close();
}

void Foundation::TimerImpl::Open(uv_loop_t * loop, void * data) {
	if (!handle_) {
		handle_ = static_cast<uv_timer_t *>(malloc(sizeof(uv_timer_t)));
		uv_timer_init(loop, handle_);
	}
	handle_->data = data;
}

void Foundation::TimerImpl::Close(uv_close_cb cb = reinterpret_cast<uv_close_cb>(free)) {
	if (handle_) {
		if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(handle_))) {
			uv_close(reinterpret_cast<uv_handle_t *>(handle_), cb);
		}
		handle_ = nullptr;
	}
}

int Foundation::TimerImpl::SetTimeout(int timeout, int repeat, uv_timer_cb cb) {
	int status = UV_UNKNOWN;
	if (handle_) {
		status = uv_timer_start(handle_, cb, timeout, repeat);
	}
	return status;
}

int Foundation::TimerImpl::Cancel() {
	int status = UV_UNKNOWN;
	if (handle_) {
		status = uv_timer_stop(handle_);
	}
	return status;
}