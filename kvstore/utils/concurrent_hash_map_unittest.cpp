#include "concurrent_hash_map.h"
#include "gtest/gtest.h"

using kvstore::ConcurrentHashMap;

namespace {
class ConcurrentHashMapTest : public testing::Test {
 protected:
  void SetUp() override {
    concurrent_hash_map_.Put("hello", "world");
    concurrent_hash_map_.Persist("test_data");
  }

  ConcurrentHashMap concurrent_hash_map_;
};

// Test Get
TEST_F(ConcurrentHashMapTest, GetSuccess) {
  auto s = concurrent_hash_map_.Get("hello");
  ASSERT_EQ(s.value(), "world");
}

// Test Get but key does not exist
TEST_F(ConcurrentHashMapTest, GetFail) {
  auto s = concurrent_hash_map_.Get("Not Exist");
  ASSERT_EQ(s.has_value(), false);
}

// Test Put
TEST_F(ConcurrentHashMapTest, Put) {
  bool res = concurrent_hash_map_.Put("CSCI", "499");
  ASSERT_EQ(res, true);
}

// Test Remove
TEST_F(ConcurrentHashMapTest, Remove) {
  bool res = concurrent_hash_map_.Remove("hello");
  ASSERT_EQ(res, true);
}

// Test Load
TEST_F(ConcurrentHashMapTest, Load) {
  bool res = concurrent_hash_map_.Load("test_data");
  ASSERT_EQ(res, true);
  ASSERT_EQ(concurrent_hash_map_.Get("hello"), "world");
}

// Test Persist
TEST_F(ConcurrentHashMapTest, Persist) {
  bool res = concurrent_hash_map_.Persist("test_data");
  ASSERT_EQ(res, true);
}
}  // namespace
