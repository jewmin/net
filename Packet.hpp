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

#ifndef Foundation_Packet_INCLUDED
#define Foundation_Packet_INCLUDED

#include "PacketReader.hpp"
#include <algorithm>

namespace Foundation {
	class Packet : public PacketReader {
	public:
		Packet() : PacketReader(), alloc_size_(0), used_size_(0) {
		}

		Packet(char * buf, int size) : PacketReader(buf, size), alloc_size_(0), used_size_(0) {
			data_end_ = mem_ptr_;
		}

		virtual ~Packet() {
			Free();
		}

		// 设置内存增长大小
		inline void SetAllocSize(int size) {
			alloc_size_ = size;
		}

		// 获取当前缓冲长度下的可用字节数
		inline int GetWritableLength() {
			return end_ptr_ - offset_;
		}

		// 设置数据包当前读写位置字节偏移量，如果新的偏移量比当前内存块长度要大，则函数会与SetLength一样进行内存块的扩展操作
		virtual int SetPosition(int new_pos) {
			if (new_pos > GetLength()) {
				Realloc(new_pos);
			}
			offset_ = mem_ptr_ + new_pos;
			// 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
			if (offset_ > data_end_) {
				data_end_ = offset_;
			}
			return new_pos;
		}

		// 基于当前读写指针偏移位置调整偏移量
		// adjust_offset要调整的偏移量大小，负数则表示向内存开始处调整（降低偏移）
		// 函数内会限制调整后的偏移位置必须大于等于内存开头，如果调整后的偏移超过现有内存长度，则会自动增长内存到欲求调整的位置
		virtual int AdjustOffset(int adjust_offset) {
			offset_ += adjust_offset;
			if (offset_ < mem_ptr_) {
				offset_ = mem_ptr_;
			} else if (offset_ > end_ptr_) {
				Realloc(offset_ - mem_ptr_);
			}
			// 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
			if (offset_ > data_end_) {
				data_end_ = offset_;
			}
			return offset_ - mem_ptr_;
		}

		// 设置数据长度
		inline void SetDataLength(int new_length) {
			if (new_length > GetLength()) {
				Realloc(new_length);
			}
			data_end_ = mem_ptr_ + new_length;
			if (offset_ > data_end_) {
				offset_ = data_end_;
			}
		}

		inline int Reserve(int new_size) {
			int mem_size = GetLength();
			if (mem_size < new_size) {
				Realloc(new_size);
			}
			return mem_size;
		}

		// 释放内存
		void Free() {
			if (mem_ptr_ && used_size_ > 0) {
				free(mem_ptr_);
				mem_ptr_ = end_ptr_ = offset_ = data_end_ = nullptr;
				used_size_ = 0;
			}
		}

		// 写入原子数据
		template<typename T>
		inline void WriteAtom(const T & data) {
			int mem_size = GetWritableLength();
			if (mem_size < sizeof(data)) {
				Realloc(GetLength() + sizeof(data));
			}
			*(reinterpret_cast<T *>(offset_)) = data;
			offset_ += sizeof(data);
			// 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
			if (offset_ > data_end_) {
				data_end_ = offset_;
			}
		}

		// 写入二进制数据
		inline void WriteBinary(const char * buf, int size) {
			int mem_size = GetWritableLength();
			if (mem_size < size) {
				Realloc(GetLength() + size);
			}
			memcpy(offset_, buf, size);
			offset_ += size;
			// 如果读写指针超过长度指针的位置，则调整长度指针为读写指针的位置
			if (offset_ > data_end_) {
				data_end_ = offset_;
			}
		}

		// 写入字符串数据，字符串的数据格式为：[2字节字符长度数据][字符串字节数据，字符串长度在65536以内][字符串终止字符0]
		inline void WriteString(const char * str, int len = -1) {
			RawWriteStringLength<u16>(str, len);
		}

		template<typename T>
		inline Packet & operator << (const T & value) {
			if (sizeof(value) <= sizeof(int)) {
				WriteAtom<T>(value);
			} else {
				WriteBinary(reinterpret_cast<const char *>(&value), sizeof(value));
			}
			return *this;
		}

		inline Packet & operator << (const char * value) {
			WriteString(value, strlen(value));
			return *this;
		}

		inline Packet & operator << (char * value) {
			WriteString(value, strlen(value));
			return *this;
		}

	protected:
		// 重新设定数据包内存空间大小
		void Realloc(int new_size) {
			new_size += alloc_size_;
			char * old_mem = mem_ptr_;
			char * new_mem = static_cast<char *>(malloc(new_size));
			int offset = GetPosition();
			int length = GetDataLength();
			if (length > 0) {
				memcpy(new_mem, mem_ptr_, std::min<int>(new_size, length));
			}
			// 重设数据指针
			mem_ptr_ = new_mem;
			end_ptr_ = mem_ptr_ + new_size;
			offset_ = mem_ptr_ + offset;
			data_end_ = mem_ptr_ + length;
			// 销毁原有数据内存
			if (old_mem && used_size_ > 0) {
				free(old_mem);
			}
			// 设置分配内存大小
			used_size_ = new_size;
		}

		// 写入字符串数据，字符串的数据格式为：[2字节字符长度数据][字符串字节数据，字符串长度在65536以内][字符串终止字符0]
		template<typename LENGTH>
		void RawWriteStringLength(const char * str, int len) {
			if (!str) {
				str = "";
			}
			if ((int)-1 == len) {
				len = str ? (LENGTH)strlen(str) : 0;
			} else {
				len = std::min<int>(len, strlen(str));
			}
			WriteAtom<LENGTH>((LENGTH)len);
			WriteBinary(reinterpret_cast<const char *>(str), len);
			WriteAtom<char>(0);
		}

	private:
		Packet(const Packet &) = delete;
		Packet & operator=(const Packet &) = delete;

	protected:
		int alloc_size_; // 内存增长大小
		int used_size_; // 使用内存分配器分配的内存
	};
}

#endif