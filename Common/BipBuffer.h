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

#ifndef Net_Common_BipBuffer_INCLUDED
#define Net_Common_BipBuffer_INCLUDED

#include "Common/IOBuffer.h"

namespace Net {

class BipBuffer : public IOBuffer {
public:
	BipBuffer();
	virtual ~BipBuffer();

	i8 * GetReserveBlock(i32 want_size, i32 & actually_size) override;
	i8 * GetContiguousBlock(i32 & size) override;
	void Commit(i32 size) override;
	void DeCommit(i32 size) override;
	i32 GetCommitedSize() const override;

protected:
	i32 GetSpaceAfterA() const;
	i32 GetBFreeSpace() const;

private:
	i32 offset_a_;
	i32 size_a_;
	i32 offset_b_;
	i32 size_b_;
	i32 offset_reserve_;
	i32 size_reserve_;
};

inline i32 BipBuffer::GetSpaceAfterA() const {
	return buffer_length_ - offset_a_ - size_a_;
}

inline i32 BipBuffer::GetBFreeSpace() const {
	return offset_a_ - offset_b_ - size_b_;
}

}

#endif