#include <gtest/gtest.h>
#include <common/bustub_instance.h>

namespace bustub {
class OptimizerTest : public ::testing::Test {
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


TEST_F(OptimizerTest, SortLimitAsTopNTest) {
  std::string sql = "select * from test_2 order by colA limit 10";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(sql, writer);

}
}