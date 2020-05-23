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

#ifndef Net_Common_IOBuffer_INCLUDED
#define Net_Common_IOBuffer_INCLUDED

#include "Common/NetObject.h"
#include "Common/Allocator.h"

namespace Net {

class IOBuffer : public NetObject {
public:
	IOBuffer() : buffer_(nullptr), buffer_length_(0) {}
	virtual ~IOBuffer() { DeAllocate(); }

	void Allocate(i32 size) {
		if (!buffer_) {
			buffer_ = static_cast<i8 *>(jc_malloc(size));
			buffer_length_ = size;
		}
	}

	i32 Write(const i8 * data, i32 data_len) {
		i32 actually_size = 0;
		i8 * block = GetReserveBlock(data_len, actually_size);
		if (block && actually_size > 0) {
			std::memcpy(block, data, actually_size);
			Commit(actually_size);
			return actually_size;
		}
		return UV_ENOBUFS;
	}

	i32 Read(i8 * data, i32 data_len) {
		i32 actually_size = 0;
		i8 * block = GetContiguousBlock(actually_size);
		if (block && actually_size > 0) {
			if (actually_size < data_len) {
				data_len = actually_size;
			}
			std::memcpy(data, block, data_len);
			DeCommit(data_len);
			return data_len;
		}
		return 0;
	}

	virtual i8 * GetReserveBlock(i32 want_size, i32 & actually_size) = 0;
	virtual i8 * GetContiguousBlock(i32 & size) = 0;
	virtual void Commit(i32 size) = 0;
	virtual void DeCommit(i32 size) = 0;
	virtual i32 GetCommitedSize() const = 0;
	virtual i32 GetFreeSize() const = 0;

protected:
	void DeAllocate() {
		if (buffer_) {
			jc_free(buffer_);
			buffer_ = nullptr;
			buffer_length_ = 0;
		}
	}

private:
	IOBuffer(IOBuffer &&) = delete;
	IOBuffer(const IOBuffer &) = delete;
	IOBuffer & operator=(IOBuffer &&) = delete;
	IOBuffer & operator=(const IOBuffer &) = delete;

protected:
	i8 * buffer_;
	i32 buffer_length_;
};

}

#endif