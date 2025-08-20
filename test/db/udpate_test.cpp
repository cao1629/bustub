
#include "gtest/gtest.h"
#include "test_util2.h"

namespace bustub {

TEST(UpdateTest, Test2) {
  std::string sql = "update test_simple_seq_1 set col1 = 100 where col1 < 10;";
  ExecuteSqlString(sql);
}
}