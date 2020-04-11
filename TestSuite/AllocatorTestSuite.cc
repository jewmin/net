#include "gtest/gtest.h"
#include "Common/Allocator.h"

TEST(AllocatorTestSuite, replace) {
	EXPECT_EQ(false, jc_replace_allocator(std::malloc, nullptr, nullptr, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(nullptr, std::realloc, nullptr, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(nullptr, nullptr, std::calloc, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(nullptr, nullptr, nullptr, std::free));
	EXPECT_EQ(false, jc_replace_allocator(std::malloc, std::realloc, nullptr, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(std::malloc, std::realloc, std::calloc, nullptr));
	EXPECT_EQ(true, jc_replace_allocator(std::malloc, std::realloc, std::calloc, std::free));
}