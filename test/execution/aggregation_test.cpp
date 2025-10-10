
#include <common/bustub_instance.h>
#include <gtest/gtest.h>

namespace bustub {

class AggregationTest : public ::testing::Test {
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

TEST_F(AggregationTest, CountStarTest) {
  std::string sql = "select count(*) from test_2 group by colC";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(sql, writer);
}

TEST_F(AggregationTest, CountTest) {
  std::string sql = "select count(colA) from test_2 group by colC";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(sql, writer);
}

TEST_F(AggregationTest, MaxTest) {
  std::string sql = "select max(colA) from test_2 group by colC";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(sql, writer);
}

TEST_F(AggregationTest, SumTest) {
  std::string sql = "select sum(colA) from test_2 group by colC";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(sql, writer);
}

TEST_F(AggregationTest, FilterTest) {
  std::string sql = "select max(colA) from test_2 group by colC having min(colB) > 10";
  NoopWriter writer;
  bustub_instance_->ExecuteSql(sql, writer);
}

}  // namespace bustub