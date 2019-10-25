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

#ifndef Timer_Timer_INCLUDED
#define Timer_Timer_INCLUDED

#include "TimerImpl.h"

namespace Foundation {
	class Timer {
	public:
		Timer();
		Timer(const Timer & rhs);
		Timer & operator=(const Timer & rhs);
		virtual ~Timer();

		void Open(uv_loop_t * loop, void * data);
		void Close(uv_close_cb cb = reinterpret_cast<uv_close_cb>(free));
		// params timeout: 经过多少毫秒后到期
		// params repeat: timeout到期后，之后每次经过多少毫秒后到期
		int SetTimeout(int timeout, int repeat, uv_timer_cb cb);
		int Cancel();
		TimerImpl * Impl() const;

		bool operator==(const Timer & rhs) const;
		bool operator!=(const Timer & rhs) const;
		bool operator<(const Timer & rhs) const;
		bool operator<=(const Timer & rhs) const;
		bool operator>(const Timer & rhs) const;
		bool operator>=(const Timer & rhs) const;

	protected:
		explicit Timer(TimerImpl * impl);

	private:
		TimerImpl * impl_;
	};
}

inline void Foundation::Timer::Open(uv_loop_t * loop, void * data) {
	impl_->Open(loop, data);
}

inline void Foundation::Timer::Close(uv_close_cb cb = reinterpret_cast<uv_close_cb>(free)) {
	impl_->Close(cb);
}

inline int Foundation::Timer::SetTimeout(int timeout, int repeat, uv_timer_cb cb) {
	return impl_->SetTimeout(timeout, repeat, cb);
}

inline int Foundation::Timer::Cancel() {
	return impl_->Cancel();
}

inline Foundation::TimerImpl * Foundation::Timer::Impl() const {
	return impl_;
}

inline bool Foundation::Timer::operator==(const Timer & rhs) const {
	return impl_ == rhs.impl_;
}

inline bool Foundation::Timer::operator!=(const Timer & rhs) const {
	return impl_ != rhs.impl_;
}

inline bool Foundation::Timer::operator<(const Timer & rhs) const {
	return impl_ < rhs.impl_;
}

inline bool Foundation::Timer::operator<=(const Timer & rhs) const {
	return impl_ <= rhs.impl_;
}

inline bool Foundation::Timer::operator>(const Timer & rhs) const {
	return impl_ > rhs.impl_;
}

inline bool Foundation::Timer::operator>=(const Timer & rhs) const {
	return impl_ >= rhs.impl_;
}

#endif