#include "gtest/gtest.h"
#include "Common/ObjectMgr.h"

class TestObj {
public:
	TestObj(int value) : v(value) {}
	virtual ~TestObj() {}
	int v;
};

class TestObjMgr : public Net::ObjectMgr<TestObj> {
public:
	TestObjMgr(u32 size) : Net::ObjectMgr<TestObj>(size) {}
	virtual ~TestObjMgr() {}
	using Net::ObjectMgr<TestObj>::FFS;
};

TEST(ObjectMgrTestSuite, ctor) {
	Net::ObjectMgr<TestObj> mgr1(100);
	Net::ObjectMgr<TestObj> mgr2(128);
}

TEST(ObjectMgrTestSuite, add) {
	TestObj * o = new TestObj(0);
	Net::ObjectMgr<TestObj> mgr1(100);
	for (int i = 0; i < 100; ++i) {
		if (i >= 64) {
			EXPECT_EQ(mgr1.AddNewObj(o), -1);
			EXPECT_EQ(mgr1.GetObjCount(), 64);
		} else {
			EXPECT_EQ(mgr1.AddNewObj(o), i);
			EXPECT_EQ(mgr1.GetObjCount(), i + 1);
		}
	}
	Net::ObjectMgr<TestObj> mgr2(128);
	for (int i = 0; i < 128; ++i) {
		EXPECT_EQ(mgr2.AddNewObj(o), i);
	}
	EXPECT_FALSE(mgr2.GetObj(0) == nullptr);
	EXPECT_FALSE(mgr2.GetObj(4) == nullptr);
	EXPECT_FALSE(mgr2.GetObj(23) == nullptr);
	EXPECT_FALSE(mgr2.GetObj(60) == nullptr);
	EXPECT_FALSE(mgr2.GetObj(127) == nullptr);
	EXPECT_TRUE(mgr2.GetObj(128) == nullptr);
	EXPECT_TRUE(mgr2.GetObj(200) == nullptr);
	EXPECT_TRUE(mgr2.GetObj(-1) == nullptr);
	delete o;
}

TEST(ObjectMgrTestSuite, ffs) {
	TestObjMgr mgr(64);
	EXPECT_EQ(mgr.FFS(2233922225252), 2);
	EXPECT_EQ(mgr.FFS(2233922225248), 5);
	EXPECT_EQ(mgr.FFS(2233922225152), 10);
	EXPECT_EQ(mgr.FFS(2233922256896), 15);
	EXPECT_EQ(mgr.FFS(2233922224128), 18);
	EXPECT_EQ(mgr.FFS(2233921961984), 21);
	EXPECT_EQ(mgr.FFS(2233986973696), 26);
	EXPECT_EQ(mgr.FFS(2233919864832), 29);
	EXPECT_EQ(mgr.FFS(2233382993920), 35);
	EXPECT_EQ(mgr.FFS(2336462209024), 37);
	EXPECT_EQ(mgr.FFS(2199023255552), 41);
	EXPECT_EQ(mgr.FFS(140737488355328), 47);
	EXPECT_EQ(mgr.FFS(4909205068810551296), 48);
	EXPECT_EQ(mgr.FFS(4908923593833840640), 53);
	EXPECT_EQ(mgr.FFS(4899916394579099648), 58);
	EXPECT_EQ(mgr.FFS(4611686018427387904), 62);
}

TEST(ObjectMgrTestSuite, rm) {
	TestObj * o = new TestObj(0);
	Net::ObjectMgr<TestObj> mgr1(128);
	for (int i = 0; i < 128; ++i) {
		mgr1.AddNewObj(o);
	}
	for (int i = 0; i < 128; i += 2) {
		EXPECT_FALSE(mgr1.RemoveObj(i) == nullptr);
	}
	for (int i = 0; i < 128; i += 2) {
		EXPECT_EQ(mgr1.AddNewObj(o), i);
	}
	delete o;
}

void Func(TestObj * object, void * ud) {
	int * sum = (int *)ud;
	*sum += object->v;
}

TEST(ObjectMgrTestSuite, visit) {
	Net::ObjectMgr<TestObj> mgr1(128);
	for (int i = 0; i < 100; ++i) {
		mgr1.AddNewObj(new TestObj(i + 1));
	}
	int sum = 0;
	mgr1.VisitObj(Func, (void *)&sum);
	EXPECT_EQ(sum, 5050);
	for (int i = 0; i < 100; ++i) {
		delete mgr1.RemoveObj(i);
	}
}

TEST(ObjectMgrTestSuite, crash) {
	try {
		Net::ObjectMgr<TestObj> mgr1(63);
	} catch (std::exception & e) {
		std::printf("ObjectMgrTestSuite - crash: %s\n", e.what());
	}

	try {
		Net::ObjectMgr<TestObj> mgr1(64);
		mgr1.AddNewObj(nullptr);
	} catch (std::exception & e) {
		std::printf("ObjectMgrTestSuite - crash: %s\n", e.what());
	}

	try {
		Net::ObjectMgr<TestObj> mgr1(64);
		mgr1.RemoveObj(-10);
	} catch (std::exception & e) {
		std::printf("ObjectMgrTestSuite - crash: %s\n", e.what());
	}
}