#include "gtest/gtest.h"
#include "ObjectMgr.h"

using namespace Foundation;

class TestObj {
public:
	TestObj(int value) : value_(value) {}
	virtual ~TestObj() {}
	int GetValue() const { return value_; }

private:
	int value_;
};

TEST(ObjectMgrTestSuite, Construct) {
	ObjectMgr<TestObj> mgr1(5, 8);
	ObjectMgr<TestObj> mgr2(50, 8);
	ObjectMgr<TestObj> mgr3(500, 8);
	ObjectMgr<TestObj> mgr4(5000, 8);
}

TEST(ObjectMgrTestSuite, Add) {
	ObjectMgr<TestObj> mgr1(5, 8);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(1)), 1);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(2)), 2);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(3)), 3);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(4)), 4);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(5)), 5);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(6)), 6);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(7)), 7);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(8)), 8);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(9)), 9);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(10)), 10);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(11)), 11);
	EXPECT_EQ(mgr1.AddNewObj(new TestObj(12)), 12);
}

TEST(ObjectMgrTestSuite, Add2) {
	ObjectMgr<TestObj> mgr1(5, 8);
	EXPECT_EQ(mgr1.AddNewObj(1, new TestObj(1)), 1);
	EXPECT_EQ(mgr1.AddNewObj(5, new TestObj(2)), 5);
	EXPECT_EQ(mgr1.AddNewObj(10, new TestObj(3)), 10);
	EXPECT_EQ(mgr1.AddNewObj(15, new TestObj(4)), 15);
	EXPECT_EQ(mgr1.AddNewObj(20, new TestObj(5)), 20);
	EXPECT_EQ(mgr1.AddNewObj(32, new TestObj(6)), 32);
	EXPECT_EQ(mgr1.AddNewObj(64, new TestObj(7)), 64);
	EXPECT_EQ(mgr1.AddNewObj(128, new TestObj(8)), 128);
	EXPECT_EQ(mgr1.AddNewObj(256, new TestObj(9)), 256);
	EXPECT_EQ(mgr1.AddNewObj(512, new TestObj(10)), 512);
	EXPECT_EQ(mgr1.AddNewObj(1024, new TestObj(11)), 1024);
	EXPECT_EQ(mgr1.AddNewObj(2000, new TestObj(12)), 2000);
}

TEST(ObjectMgrTestSuite, Get) {
	ObjectMgr<TestObj> mgr1(5, 8);
	mgr1.AddNewObj(1, new TestObj(1));
	mgr1.AddNewObj(5, new TestObj(2));
	mgr1.AddNewObj(10, new TestObj(3));
	mgr1.AddNewObj(15, new TestObj(4));
	mgr1.AddNewObj(20, new TestObj(5));
	mgr1.AddNewObj(32, new TestObj(6));
	mgr1.AddNewObj(64, new TestObj(7));
	mgr1.AddNewObj(128, new TestObj(8));
	mgr1.AddNewObj(256, new TestObj(9));
	mgr1.AddNewObj(512, new TestObj(10));
	mgr1.AddNewObj(1024, new TestObj(11));
	mgr1.AddNewObj(2000, new TestObj(12));

	EXPECT_EQ(mgr1.GetObj(500), nullptr);
	EXPECT_NE(mgr1.GetObj(1024), nullptr);
	EXPECT_EQ(mgr1.GetObjCount(), 12);
}

TEST(ObjectMgrTestSuite, Remove) {
	ObjectMgr<TestObj> mgr1(5, 8);
	mgr1.AddNewObj(1, new TestObj(1));
	mgr1.AddNewObj(5, new TestObj(2));
	mgr1.AddNewObj(10, new TestObj(3));
	mgr1.AddNewObj(15, new TestObj(4));
	mgr1.AddNewObj(20, new TestObj(5));
	mgr1.AddNewObj(32, new TestObj(6));
	mgr1.AddNewObj(64, new TestObj(7));
	mgr1.AddNewObj(128, new TestObj(8));
	mgr1.AddNewObj(256, new TestObj(9));
	mgr1.AddNewObj(512, new TestObj(10));
	mgr1.AddNewObj(1024, new TestObj(11));
	mgr1.AddNewObj(2000, new TestObj(12));

	TestObj * obj = mgr1.RemoveObj(128);
	EXPECT_EQ(obj->GetValue(), 8);
	delete obj;
	obj = mgr1.RemoveObj(500);
	EXPECT_EQ(obj, nullptr);

	EXPECT_EQ(mgr1.DeleteObj(200), false);
	EXPECT_EQ(mgr1.DeleteObj(256), true);

	EXPECT_EQ(mgr1.GetObj(256), nullptr);
	EXPECT_EQ(mgr1.DeleteObj(256), false);

	mgr1.AddNewObj(200, new TestObj(13));
	mgr1.AddNewObj(256, new TestObj(14));
}

void Func(void * object, void * ud) {
	int * sum = static_cast<int *>(ud);
	*sum += static_cast<TestObj *>(object)->GetValue();
}

TEST(ObjectMgrTestSuite, Each) {
	ObjectMgr<TestObj> mgr1(5, 8);
	mgr1.AddNewObj(1, new TestObj(1));
	mgr1.AddNewObj(5, new TestObj(2));
	mgr1.AddNewObj(10, new TestObj(3));
	mgr1.AddNewObj(15, new TestObj(4));
	mgr1.AddNewObj(20, new TestObj(5));
	mgr1.AddNewObj(32, new TestObj(6));
	mgr1.AddNewObj(64, new TestObj(7));
	mgr1.AddNewObj(128, new TestObj(8));
	mgr1.AddNewObj(256, new TestObj(9));
	mgr1.AddNewObj(512, new TestObj(10));
	mgr1.AddNewObj(1024, new TestObj(11));
	mgr1.AddNewObj(2000, new TestObj(12));

	int sum = 0;
	mgr1.EnumEachObj(Func, static_cast<void *>(&sum));
	EXPECT_EQ(sum, 78);
}

TEST(ObjectMgrTestSuite, Catch) {
	ObjectMgr<TestObj> mgr1(5, 8);
	EXPECT_EQ(mgr1.GetObj(0), nullptr);
	EXPECT_EQ(mgr1.GetObj(10), nullptr);

	try {
		mgr1.AddNewObj(nullptr);
	} catch (std::exception & e) {
		printf("ObjectMgrTestSuite - Catch: %s\n", e.what());
	}

	try {
		mgr1.AddNewObj(0, nullptr);
	} catch (std::exception & e) {
		printf("ObjectMgrTestSuite - Catch: %s\n", e.what());
	}

	try {
		mgr1.AddNewObj(10, nullptr);
	} catch (std::exception & e) {
		printf("ObjectMgrTestSuite - Catch: %s\n", e.what());
	}

	TestObj * obj = new TestObj(1);
	mgr1.AddNewObj(1, obj);
	try {
		mgr1.AddNewObj(1, obj);
	} catch (std::exception & e) {
		printf("ObjectMgrTestSuite - Catch: %s\n", e.what());
	}

	try {
		mgr1.RemoveObj(0);
	} catch (std::exception & e) {
		printf("ObjectMgrTestSuite - Catch: %s\n", e.what());
	}

	try {
		mgr1.RemoveObj(10);
	} catch (std::exception & e) {
		printf("ObjectMgrTestSuite - Catch: %s\n", e.what());
	}
}