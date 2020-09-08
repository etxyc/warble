#include "warble_funcs.h"
#include "kvstore_client.h"
#include "gtest/gtest.h"

using func::KeyValueStoreClient;

enum EventIdName {
  kRegisterUser = 1,
  kWarble = 2,
  kFollow = 3,
  kRead = 4,
  kProfile = 5,
  kHookAll = 7,
  kUnhookAll = 8,
  kReply = 9
};

namespace {
class WarbleFuncsTest : public testing::Test {
 protected:
  void SetUp() override {

  }
};

// Test RegisterUser
TEST_F(WarbleFuncsTest , Registeruser) {
  RegisteruserRequest registeruser_request;
  registeruser_request.set_username("Test_User");
  EventRequest request;
  EventReply reply;
  request.set_event_type(kRegisterUser);
  request.mutable_payload()->PackFrom(registeruser_request);

  func::KeyValueStoreClient key_value_store_client_(grpc::CreateChannel("localhost:50001",
      grpc::InsecureChannelCredentials()));

  bool check = warble_funcs::RegisterUser(request, reply, key_value_store_client_);
  ASSERT_EQ(check, true);

}

// Test Profile
TEST_F(WarbleFuncsTest , Profile) {
  ProfileRequest profile_request;
  profile_request.set_username("Test_User");

  EventRequest request;
  EventReply reply;
  request.set_event_type(kProfile);
  request.mutable_payload()->PackFrom(profile_request);

  func::KeyValueStoreClient key_value_store_client_(grpc::CreateChannel("localhost:50001",
      grpc::InsecureChannelCredentials()));

  bool check = warble_funcs::Profile(request, reply, key_value_store_client_);
  ASSERT_EQ(check, true);
}

// Test Warble
TEST_F(WarbleFuncsTest , Warble) {
  WarbleRequest warble_request;
  warble_request.set_username("Test_User");
  warble_request.set_text("Test_text");
  warble_request.set_parent_id("-1");

  EventRequest request;
  EventReply reply;
  request.set_event_type(kWarble);
  request.mutable_payload()->PackFrom(warble_request);

  func::KeyValueStoreClient key_value_store_client_(grpc::CreateChannel("localhost:50001",
      grpc::InsecureChannelCredentials()));

  bool check = warble_funcs::Warble(request, reply, key_value_store_client_);
  ASSERT_EQ(check, true);
  key_value_store_client_.Remove("Test_User");
}
}  // namespace
