#include "gtest/gtest.h"
#include "NetworkException.h"

TEST(NetworkExceptionTestSuite, message_ctor) {
	try {
		throw Net::NetworkException("default");
	} catch (Common::CExpection & e) {
		EXPECT_STREQ(e.What(), "default");
		std::printf("NetworkException: %s\n", e.What());
	}
}