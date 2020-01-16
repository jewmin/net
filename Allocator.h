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

#ifndef Net_Allocator_INCLUDED
#define Net_Allocator_INCLUDED

#include "Net.h"

namespace Net {

class Allocator {
public:
protected:
	static const i32 SmallAlign = 8;
	static const i32 SmallMaxBytes = 256;
	static const i32 SmallFreeListSize = SmallMaxBytes / SmallAlign;
	static const i32 LargeAlign = 64;
	static const i32 LargeMaxBytes = 2048;
	static const i32 LargeFreeListSize = LargeMaxBytes / LargeAlign;

private:
	i32 SmallSizeClass(i32 size);
	i32 LargeSizeClass(i32 size);
	i32 ClassIndex(i32 size);
};

inline i32 Allocator::SmallSizeClass(i32 size) {
	return size >> 3;
}

inline i32 Allocator::LargeSizeClass(i32 size) {
	return (size + (28 << 7)) >> 7;
}

inline i32 Allocator::ClassIndex(i32 size) {
	if (size < SmallMaxBytes) {
		return SmallSizeClass(size);
	} else {
		return LargeSizeClass(size);
	}
}

}

#endif