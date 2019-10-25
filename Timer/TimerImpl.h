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

#ifndef Timer_TimerImpl_INCLUDED
#define Timer_TimerImpl_INCLUDED

#include "uv.h"
#include "RefCountedObject.h"

namespace Foundation {
	class TimerImpl : public RefCountedObject {
	public:
		TimerImpl();
		virtual ~TimerImpl();
		TimerImpl(const TimerImpl &) = delete;
		TimerImpl & operator=(const TimerImpl &) = delete;

		virtual void Open(uv_loop_t * loop, void * data);
		virtual void Close(uv_close_cb cb = reinterpret_cast<uv_close_cb>(free));
		virtual int SetTimeout(int timeout, int repeat, uv_timer_cb cb);
		virtual int Cancel();

	private:
		uv_timer_t * handle_;
	};
}

#endif