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

#include "Timer.h"

Foundation::Timer::Timer() : impl_(new TimerImpl()) {
}

Foundation::Timer::Timer(TimerImpl * impl) : impl_(impl) {
}

Foundation::Timer::Timer(const Timer & rhs) : impl_(rhs.impl_) {
	impl_->Duplicate();
}

Foundation::Timer & Foundation::Timer::operator=(const Timer & rhs) {
	if (this != &rhs) {
		if (impl_) {
			impl_->Release();
		}
		impl_ = rhs.impl_;
		if (impl_) {
			impl_->Duplicate();
		}
	}
	return *this;
}

Foundation::Timer::~Timer() {
	impl_->Release();
}