

#include <common/bustub_instance.h>
#include <gtest/gtest.h>

namespace bustub {
class JoinTest : public ::testing::Test {
 protected:
  void SetUp() override {
    bustub_instance_ = std::make_unique<BustubInstance>("test.db");
    bustub_instance_->GenerateMockTable();
    if (bustub_instance_->buffer_pool_manager_ != nullptr) {
      bustub_instance_->GenerateTestTable();
    }
  }

  void TearDown() override {}

  std::unique_ptr<BustubInstance> bustub_instance_;
};

TEST_F(JoinTest, NestedLoopJoinTest) {
  std::string create_index_sql = "create index test3_colA on test_3(colA)";
  std::string sql = "select * from test_2, test_3 where test_2.colA = test_3.colA";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(create_index_sql, writer);
  bustub_instance_->ExecuteSql(sql, writer);
}

};  // namespace bustub