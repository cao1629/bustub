#include "gtest/gtest.h"
#include "test_util2.h"

namespace bustub {

TEST(InsertTest, Test1) {
  std::string sql = "insert into test_simple_seq_1 values (100);"
                    "insert into test_simple_seq_1 values (200);"
                    "insert into test_simple_seq_1 values (300);";
  ExecuteSqlString(sql);
}
}