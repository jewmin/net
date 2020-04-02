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

#include "Common/StraightBuffer.h"

namespace Net {

StraightBuffer::StraightBuffer() : offset_(0), size_(0), size_reserve_(0) {
}

StraightBuffer::~StraightBuffer() {
}

i8 * StraightBuffer::GetReserveBlock(i32 want_size, i32 & actually_size) {
	if (size_ >= buffer_length_) {
		actually_size = 0;
		return nullptr;
	}

	if (buffer_length_ - offset_ - size_ >= want_size) {
		size_reserve_ = actually_size = want_size;
		return buffer_ + offset_ + size_;
	}

	if (offset_ > 0) {
		memmove(buffer_, buffer_ + offset_, size_);
		offset_ = 0;
	}

	if (buffer_length_ - size_ < want_size) {
		want_size = buffer_length_ - size_;
	}
	size_reserve_ = actually_size = want_size;
	return buffer_ + size_;
}

i8 * StraightBuffer::GetContiguousBlock(i32 & size) {
	if (size_ <= 0) {
		size = 0;
		return nullptr;
	}

	size = size_;
	return buffer_ + offset_;
}

void StraightBuffer::Commit(i32 size) {
	if (size <= 0) {
		size_reserve_ = 0;
		return;
	} else if (size > size_reserve_) {
		size = size_reserve_;
	}

	size_ += size;
	size_reserve_ = 0;
}

void StraightBuffer::DeCommit(i32 size) {
	if (size >= size_) {
		offset_ = 0;
		size_ = 0;
	} else {
		offset_ += size;
		size_ -= size;
	}
}

i32 StraightBuffer::GetCommitedSize() const {
	return size_;
}

}