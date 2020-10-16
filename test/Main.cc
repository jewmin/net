#include "gtest/gtest.h"
#ifdef USE_VLD
#include "vld.h"
#endif
#include "Category.h"
#include "Appender/FileAppender.h"
#include "Layout/PatternLayout.h"

int main(int argc, char * * argv) {
	testing::InitGoogleTest(&argc, argv);
	Logger::PatternLayout * layout = new Logger::PatternLayout();
	layout->SetConversionPattern("%d{%Y-%m-%d %H:%M:%S,%l} - unittest - net - %c - %P - %m%n");
	Logger::FileAppender * appender = new Logger::FileAppender("unittest-net", "unittest-net.log", true);
	appender->SetLayout(layout);
	Logger::Category::GetCategory("SocketImpl")->AddAppender(appender);
	return RUN_ALL_TESTS();
}