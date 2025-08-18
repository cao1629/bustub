#include "gtest/gtest.h"
#include <iostream>
#include "test_util2.h"

namespace bustub {
TEST(SelectTest, Test1) {
  std::string sql = "select * from test_simple_seq_1";
  ExecuteSqlString(sql);
}
}