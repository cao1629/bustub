#pragma once

#include <iostream>
#include "common/bustub_instance.h"

auto ExecuteSqlString(const std::string &sql) -> void {
  auto bustub = std::make_unique<bustub::BustubInstance>("test.db");
  if (bustub->buffer_pool_manager_ != nullptr) {
    bustub->GenerateTestTable();
  }

  try {
    auto writer = bustub::FortTableWriter();
    bustub->ExecuteSql(sql, writer);
    for (const auto &table : writer.tables_) {
      std::cout << table;
    }
  } catch (bustub::Exception &ex) {
    std::cerr << ex.what() << std::endl;
  }
}