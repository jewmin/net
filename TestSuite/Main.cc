#include "gtest/gtest.h"
#ifdef USE_VLD
#include "vld.h"
#endif
#include "Net.h"

void MyAbort() {
	throw std::runtime_error("MyAbort");
}

int main(int argc, char * * argv) {
	std::printf("sizeof(i8)=%zu sizeof(i16)=%zu sizeof(i32)=%zu sizeof(i64)=%zu sizeof(ptr)=%zu\n", sizeof(i8), sizeof(i16), sizeof(i32), sizeof(i64), sizeof(void *));
	std::printf("sizeof(u8)=%zu sizeof(u16)=%zu sizeof(u32)=%zu sizeof(u64)=%zu\n", sizeof(u8), sizeof(u16), sizeof(u32), sizeof(u64));
	testing::InitGoogleTest(&argc, argv);
	jc_replace_abort(MyAbort);
	int result = RUN_ALL_TESTS();
	return result;
}