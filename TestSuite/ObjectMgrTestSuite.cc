#include "gtest/gtest.h"
#define private public
#define protected public
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
};

class ObjectMgrTestSuite : public testing::Test {
public:
	ObjectMgrTestSuite() : mgr_(nullptr), size_(200) {}
	// Sets up the test fixture.
	virtual void SetUp() {
		mgr_ = new Net::ObjectMgr<TestObj>(size_);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete mgr_;
	}

	Net::ObjectMgr<TestObj> * mgr_;
	int size_;
};

TEST_F(ObjectMgrTestSuite, ctor) {
	EXPECT_ANY_THROW(Net::ObjectMgr<TestObj> mgr1(0));
	EXPECT_ANY_THROW(Net::ObjectMgr<TestObj> mgr2(63));
	Net::ObjectMgr<TestObj> mgr3(64);
	EXPECT_EQ((int)mgr3.kObjectBitsMaxSize, 1);
	Net::ObjectMgr<TestObj> mgr4(100);
	EXPECT_EQ((int)mgr4.kObjectBitsMaxSize, 1);
	Net::ObjectMgr<TestObj> mgr5(128);
	EXPECT_EQ((int)mgr5.kObjectBitsMaxSize, 2);
	Net::ObjectMgr<TestObj> mgr6(-1);
	EXPECT_EQ((int)mgr6.kObjectBitsMaxSize, 67108863);
}

class ObjectMgrTestSuite_Add : public ObjectMgrTestSuite {
public:
	// Sets up the test fixture.
	virtual void SetUp() {
		ObjectMgrTestSuite::SetUp();
		object_ = new TestObj(1);
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete object_;
		ObjectMgrTestSuite::TearDown();
	}

	void TestAdd() {
		for (int i = 0; i < size_; ++i) {
			if ((i >> mgr_->kBitShift) >= (int)mgr_->kObjectBitsMaxSize) {
				EXPECT_EQ(mgr_->AddNewObj(object_), -1);
				EXPECT_EQ(mgr_->GetObjCount(), mgr_->kObjectBitsMaxSize << mgr_->kBitShift);
			} else {
				EXPECT_EQ(mgr_->AddNewObj(object_), i);
				EXPECT_EQ((int)mgr_->GetObjCount(), i + 1);
			}
		}
	}

	TestObj * object_;
};

TEST_F(ObjectMgrTestSuite_Add, add) {
	TestAdd();
	EXPECT_ANY_THROW(mgr_->AddNewObj(nullptr));
	mgr_->object_bits_[0] = 0;
	EXPECT_ANY_THROW(mgr_->AddNewObj(object_));
}

class ObjectMgrTestSuite_Op : public ObjectMgrTestSuite_Add {
public:
	virtual void SetUp() {
		ObjectMgrTestSuite_Add::SetUp();
		TestAdd();
	}
};

TEST_F(ObjectMgrTestSuite_Op, get) {
	EXPECT_TRUE(mgr_->GetObj(0) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(15) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(16) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(31) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(32) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(63) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(64) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(127) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(128) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(191) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(192) == nullptr);
	EXPECT_TRUE(mgr_->GetObj(255) == nullptr);
	EXPECT_TRUE(mgr_->GetObj(256) == nullptr);
	EXPECT_TRUE(mgr_->GetObj(-1) == nullptr);
}

TEST_F(ObjectMgrTestSuite_Op, rm) {
	for (int i = 0; i < size_; i += 2) {
		if ((i >> mgr_->kBitShift) >= (int)mgr_->kObjectBitsMaxSize) {
			EXPECT_TRUE(mgr_->RemoveObj(i) == nullptr);
		} else {
			EXPECT_TRUE(mgr_->RemoveObj(i) != nullptr);
		}
	}
	for (int i = 0; i < size_ / 2; ++i) {
		if (i * 2 >= (int)mgr_->kObjectBitsMaxSize << mgr_->kBitShift) {
			EXPECT_EQ(mgr_->AddNewObj(object_), -1);
		} else {
			EXPECT_EQ(mgr_->AddNewObj(object_), i * 2);
		}
	}
	EXPECT_ANY_THROW(mgr_->RemoveObj(-1));
}

void Func(TestObj * object, void * ud) {
	int * sum = (int *)ud;
	*sum += object->v;
}

TEST_F(ObjectMgrTestSuite_Op, visit) {
	int sum = 0;
	mgr_->VisitObj(Func, (void *)&sum);
	EXPECT_EQ(sum, (int)mgr_->kObjectBitsMaxSize << mgr_->kBitShift);
}

TEST_F(ObjectMgrTestSuite_Op, ffs) {
	EXPECT_EQ(mgr_->FFS(2233922225252), 2);
	EXPECT_EQ(mgr_->FFS(2233922225248), 5);
	EXPECT_EQ(mgr_->FFS(2233922225152), 10);
	EXPECT_EQ(mgr_->FFS(2233922256896), 15);
	EXPECT_EQ(mgr_->FFS(2233922224128), 18);
	EXPECT_EQ(mgr_->FFS(2233921961984), 21);
	EXPECT_EQ(mgr_->FFS(2233986973696), 26);
	EXPECT_EQ(mgr_->FFS(2233919864832), 29);
	EXPECT_EQ(mgr_->FFS(2233382993920), 35);
	EXPECT_EQ(mgr_->FFS(2336462209024), 37);
	EXPECT_EQ(mgr_->FFS(2199023255552), 41);
	EXPECT_EQ(mgr_->FFS(140737488355328), 47);
	EXPECT_EQ(mgr_->FFS(4909205068810551296), 48);
	EXPECT_EQ(mgr_->FFS(4908923593833840640), 53);
	EXPECT_EQ(mgr_->FFS(4899916394579099648), 58);
	EXPECT_EQ(mgr_->FFS(4611686018427387904), 62);
}

#undef private
#undef protected