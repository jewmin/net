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
	TestObjMgr() : Net::ObjectMgr<TestObj>() {}
	virtual ~TestObjMgr() {}
};

class ObjectMgrTestSuite : public testing::Test {
public:
	ObjectMgrTestSuite() : mgr_(nullptr) {}
	// Sets up the test fixture.
	virtual void SetUp() {
		mgr_ = new Net::ObjectMgr<TestObj>();
	}
	// Tears down the test fixture.
	virtual void TearDown() {
		delete mgr_;
	}

	Net::ObjectMgr<TestObj> * mgr_;
};

class ObjectMgrTestSuite_Add : public ObjectMgrTestSuite {
public:
	ObjectMgrTestSuite_Add() : size_(200) {}
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
			EXPECT_EQ(mgr_->AddNewObj(object_), i);
			EXPECT_EQ((int)mgr_->GetObjCount(), i + 1);
		}
	}

	TestObj * object_;
	i32 size_;
};

TEST_F(ObjectMgrTestSuite_Add, add) {
	TestAdd();
	EXPECT_ANY_THROW(mgr_->AddNewObj(nullptr));
	EXPECT_ANY_THROW(mgr_->AddNewObjById(-1, nullptr));
	EXPECT_ANY_THROW(mgr_->AddNewObjById(0, nullptr));
	EXPECT_ANY_THROW(mgr_->AddNewObjById(0, object_));
	EXPECT_EQ(mgr_->AddNewObjById(size_, object_), size_);
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
	EXPECT_TRUE(mgr_->GetObj(192) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(size_ - 1) != nullptr);
	EXPECT_TRUE(mgr_->GetObj(size_) == nullptr);
	EXPECT_TRUE(mgr_->GetObj(-1) == nullptr);
}

TEST_F(ObjectMgrTestSuite_Op, rm) {
	for (int i = 0; i < size_; i += 2) {
		EXPECT_TRUE(mgr_->RemoveObj(i) != nullptr);
	}
	for (int i = 0; i < size_ / 2; ++i) {
		EXPECT_EQ(mgr_->AddNewObj(object_), size_ + i);
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
	EXPECT_EQ(sum, size_);
}