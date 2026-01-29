#include <gtest/gtest.h>
#include <fstream>
#include "hmac.hpp"
#include "config.hpp"

class TestFixture : public ::testing::Test {
protected:
  TestFixture() : config_(ReadStream()), hmac_(CreateHmac()) { }

  ~TestFixture() = default;

  service::config::Config ReadStream() {
    return service::config::Config("./config.json");
  }

  service::hmac::Hmac CreateHmac() {
    return service::hmac::Hmac(config_.GetSecret());
  }

  service::config::Config config_;
  service::hmac::Hmac hmac_;
  std::vector<std::string> test_vector_msgs_{"hello", "world", "Vlad", "test_signature", "qwerqewtqwtqwt", "123nlk12nl12d1", "XasDAX2AFaFFOASFAX"};
};

TEST_F(TestFixture, TestSignVerifyUnit) {

  auto sig = hmac_.Sign(std::string("hello").c_str(), 5);

  auto is_ok = hmac_.Verify("hello", std::move(sig), 5);

  ASSERT_TRUE(is_ok);

  sig.at(0) = sig.at(0) + 1;

  is_ok = hmac_.Verify("hello", std::move(sig), 5);

  ASSERT_TRUE(!is_ok);

  sig = hmac_.Sign("hello", 5);

  is_ok = hmac_.Verify("hello!", std::move(sig), 6);

  ASSERT_TRUE(!is_ok);

}

TEST_F(TestFixture, TestDetermenisticHmacUnit) {
  std::string sig;
  bool is_ok;
  for (auto i = 0; i < 10000; ++i) {
    for (auto str : test_vector_msgs_) {
      sig = hmac_.Sign(std::move(str.c_str()), str.length());

      is_ok = hmac_.Verify(std::move(str.c_str()), std::move(sig), str.length());

      ASSERT_TRUE(is_ok);
    }
  }
}