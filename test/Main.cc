#include "gtest/gtest.h"
#ifdef USE_VLD
#include "vld.h"
#endif

int main(int argc, char * * argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}