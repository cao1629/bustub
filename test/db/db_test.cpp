

#include "gtest/gtest.h"
#include "common/bustub_instance.h"

TEST(DbTest, SelectValue) {
  auto bustub = std::make_unique<bustub::BustubInstance>("test.db");

  if (bustub->buffer_pool_manager_ != nullptr) {
    bustub->GenerateTestTable();
  }

  std::string query = "SELECT 1, 2;";

  try {
    auto writer = bustub::FortTableWriter();
    bustub->ExecuteSql(query, writer);
    for (const auto &table : writer.tables_) {
      std::cout << table;
    }
  } catch (bustub::Exception &ex) {
    std::cerr << ex.what() << std::endl;
  }
}


TEST(DbTest, SelectFrom) {
  auto bustub = std::make_unique<bustub::BustubInstance>("test.db");
  bustub->GenerateMockTable();
  if (bustub->buffer_pool_manager_ != nullptr) {
    bustub->GenerateTestTable();
  }

  std::string query = "SELECT * FROM __mock_table_1;";

  try {
    auto writer = bustub::FortTableWriter();
    bustub->ExecuteSql(query, writer);
    for (const auto &table : writer.tables_) {
      std::cout << table;
    }
  } catch (bustub::Exception &ex) {
    std::cerr << ex.what() << std::endl;
  }
}


TEST(DbTest, CrossJoin) {
  auto bustub = std::make_unique<bustub::BustubInstance>("test.db");
  bustub->GenerateMockTable();
  if (bustub->buffer_pool_manager_ != nullptr) {
    bustub->GenerateTestTable();
  }

  std::string query = "SELECT * FROM __mock_table_1, __mock_table_3 WHERE colA = colE;";

  try {
    auto writer = bustub::FortTableWriter();
    bustub->ExecuteSql(query, writer);
    for (const auto &table : writer.tables_) {
      std::cout << table;
    }
  } catch (bustub::Exception &ex) {
    std::cerr << ex.what() << std::endl;
  }
}