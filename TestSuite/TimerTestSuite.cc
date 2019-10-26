#include "gtest/gtest.h"
#include "Timer/Timer.h"

using namespace Foundation;

static int count;

void TimerTestSuite_timer_cb(uv_timer_t* handle) {
	if (++count == 10) {
		Timer * timer = static_cast<Timer *>(handle->data);
		if (timer) {
			EXPECT_EQ(timer->Cancel(), 0);
		}
	}
}

void TimerTestSuite_close_cb(uv_handle_t * handle) {
	free(handle);
}

TEST(TimerTestSuite, ctor) {
	Timer timer1, timer2, timer3(timer1);
	timer2 = timer1;
}

TEST(TimerTestSuite, use) {
	count = 0;
	Timer timer;
	timer.Open(uv_default_loop(), nullptr);
	timer.Open(uv_default_loop(), reinterpret_cast<void *>(&timer));
	EXPECT_EQ(timer.SetTimeout(100, 100, TimerTestSuite_timer_cb), 0);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	timer.Close(TimerTestSuite_close_cb);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(TimerTestSuite, close) {
	Timer timer;
	timer.Open(uv_default_loop(), nullptr);
	timer.Close();
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	uv_loop_close(uv_default_loop());
}

TEST(TimerTestSuite, eq) {
	Timer timer1, timer2(timer1), timer3, timer_low, timer_high;
	EXPECT_EQ(timer1, timer2);
	EXPECT_NE(timer1, timer3);
	if (timer1 > timer3) {
		timer_low = timer3;
		timer_high = timer1;
	}
	EXPECT_GT(timer_high, timer_low);
	EXPECT_LT(timer_low, timer_high);
	EXPECT_GE(timer_high, timer_low);
	EXPECT_LE(timer_low, timer_high);
}

TEST(TimerTestSuite, impl) {
	Timer timer;
	EXPECT_TRUE(timer.Impl() > 0);
}