//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// sqllogictest.cpp
//
// Identification: tools/sqllogictest/sqllogictest.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include "argparse/argparse.hpp"
#include "common/bustub_instance.h"
#include "common/config.h"
#include "common/exception.h"
#include "common/util/string_util.h"
#include "execution/check_options.h"
#include "fmt/core.h"
#include "fmt/ranges.h"
#include "parser.h"

auto SplitLines(const std::string &lines) -> std::vector<std::string> {
  std::stringstream linestream(lines);
  std::vector<std::string> result;
  std::string line;
  while (std::getline(linestream, line, '\n')) {
    bustub::StringUtil::RTrim(&line);
    if (!line.empty()) {
      result.emplace_back(std::exchange(line, std::string{}));
    }
  }
  return result;
}

auto ResultCompare(const std::string &produced_result, const std::string &expected_result, bustub::SortMode sort_mode,
                   bool dump_diff) -> bool {
  auto a_lines = SplitLines(produced_result);
  auto b_lines = SplitLines(expected_result);
  if (sort_mode == bustub::SortMode::ROWSORT) {
    std::sort(a_lines.begin(), a_lines.end());
    std::sort(b_lines.begin(), b_lines.end());
  }
  bool cmp_result = a_lines == b_lines;
  if (!cmp_result && dump_diff) {
    std::ofstream r("result.log", std::ios_base::out | std::ios_base::trunc);
    if (!r) {
      throw bustub::Exception("cannot open file");
    }
    for (const auto &x : a_lines) {
      r << x << std::endl;
    }
    r.close();

    std::ofstream e("expected.log", std::ios_base::out | std::ios_base::trunc);
    if (!e) {
      throw bustub::Exception("cannot open file");
    }
    for (const auto &x : b_lines) {
      e << x << std::endl;
    }
    e.close();
  }

  return cmp_result;
}

auto ProcessExtraOptions(const std::string &sql, bustub::BusTubInstance &instance,
                         const std::vector<std::string> &extra_options, bool verbose,
                         std::shared_ptr<bustub::CheckOptions> &check_options) -> bool {
  for (const auto &opt : extra_options) {
    if (bustub::StringUtil::StartsWith(opt, "ensure:")) {
      std::stringstream result;
      auto writer = bustub::SimpleStreamWriter(result);
      instance.ExecuteSql("explain (o) " + sql, writer);

      if (opt == "ensure:index_scan") {
        if (!bustub::StringUtil::Contains(result.str(), "IndexScan")) {
          fmt::print("IndexScan not found\n");
          return false;
        }
      } else if (opt == "ensure:seq_scan") {
        if (bustub::StringUtil::Contains(result.str(), "IndexScan") ||
            bustub::StringUtil::ContainsAfter("OPTIMIZER", result.str(), "Filter")) {
          fmt::print("SeqScan on not indexed columns\n");
          return false;
        }
      } else if (opt == "ensure:hash_join") {
        if (bustub::StringUtil::Split(result.str(), "HashJoin").size() != 2 &&
            !bustub::StringUtil::Contains(result.str(), "Filter")) {
          fmt::print("HashJoin not found\n");
          return false;
        }
      } else if (opt == "ensure:hash_join_no_filter") {
        if (bustub::StringUtil::Split(result.str(), "HashJoin").size() != 2 ||
            bustub::StringUtil::ContainsAfter("OPTIMIZER", result.str(), "Filter")) {
          fmt::print("Push all filters into HashJoin\n");
          return false;
        }
      } else if (opt == "ensure:hash_join*2") {
        if (bustub::StringUtil::Split(result.str(), "HashJoin").size() != 3 &&
            !bustub::StringUtil::Contains(result.str(), "Filter")) {
          fmt::print("HashJoin should appear exactly twice\n");
          return false;
        }
      } else if (opt == "ensure:hash_join*3") {
        if (bustub::StringUtil::Split(result.str(), "HashJoin").size() != 4 &&
            !bustub::StringUtil::Contains(result.str(), "Filter")) {
          fmt::print("HashJoin should appear exactly thrice\n");
          return false;
        }
      } else if (opt == "ensure:topn") {
        if (!bustub::StringUtil::Contains(result.str(), "TopN")) {
          fmt::print("TopN not found\n");
          return false;
        }
        check_options->check_options_set_.emplace(bustub::CheckOption::ENABLE_TOPN_CHECK);
      } else if (opt == "ensure:topn*2") {
        if (bustub::StringUtil::Split(result.str(), "TopN").size() != 3) {
          fmt::print("TopN should appear exactly twice\n");
          return false;
        }
        check_options->check_options_set_.emplace(bustub::CheckOption::ENABLE_TOPN_CHECK);
      } else if (opt == "ensure:index_join") {
        if (!bustub::StringUtil::Contains(result.str(), "NestedIndexJoin")) {
          fmt::print("NestedIndexJoin not found\n");
          return false;
        }
      } else if (opt == "ensure:nlj_init_check") {
        if (!bustub::StringUtil::Contains(result.str(), "NestedLoopJoin")) {
          fmt::print("NestedLoopJoin not found\n");
          return false;
        }
        check_options->check_options_set_.emplace(bustub::CheckOption::ENABLE_NLJ_CHECK);
      } else if (bustub::StringUtil::StartsWith(opt, "ensure:column-pruned")) {
        auto args = bustub::StringUtil::Split(opt, ":");
        if (args.size() != 4) {
          throw bustub::NotImplementedException(fmt::format("unsupported extra option: {}", opt));
        }
        auto expected_cols_proj = std::stoi(args[2]);
        auto expected_cols_agg = std::stoi(args[3]);
        // find agg & proj plan and test if the output schema has the expected number of columns
        auto lines = bustub::StringUtil::Split(result.str(), "\n");
        for (auto &line : lines) {
          bustub::StringUtil::LTrim(&line);
          if (bustub::StringUtil::StartsWith(line, "Agg")) {
            auto cols = bustub::StringUtil::Split(line, "],");
            if (cols.size() != 3) {
              fmt::print("Agg plan wrong formatting!\n");
              return false;
            }
            for (int i = 0; i < 2; i++) {
              if (bustub::StringUtil::Count(cols[i], "\",") + 1 > static_cast<size_t>(expected_cols_agg)) {
                fmt::print("Agg wrong column pruning count!\n");
                return false;
              }
            }
            break;
          }
          if (bustub::StringUtil::StartsWith(line, "Projection")) {
            if (bustub::StringUtil::Count(line, "\",") + 1 > static_cast<size_t>(expected_cols_proj)) {
              fmt::print("Projection wrong column pruning count!\n");
              return false;
            }
          }
        }
      } else {
        throw bustub::NotImplementedException(fmt::format("unsupported extra option: {}", opt));
      }
    } else if (bustub::StringUtil::StartsWith(opt, "timing")) {
      auto args = bustub::StringUtil::Split(opt, ":");
      auto iter = args.cbegin() + 1;
      int repeat = 1;
      std::string label;
      for (; iter != args.cend(); iter++) {
        if (bustub::StringUtil::StartsWith(*iter, "x")) {
          repeat = std::stoi(std::string(iter->cbegin() + 1, iter->cend()));
        } else if (bustub::StringUtil::StartsWith(*iter, ".")) {
          label = std::string(iter->cbegin() + 1, iter->cend());
        } else {
          throw bustub::NotImplementedException(fmt::format("unsupported arg: {}", *iter));
        }
      }
      std::vector<size_t> duration;
      for (int i = 0; i < repeat; i++) {
        auto writer = bustub::NoopWriter();
        auto clock_start = std::chrono::system_clock::now();
        instance.ExecuteSql(sql, writer);
        auto clock_end = std::chrono::system_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(clock_end - clock_start);
        duration.push_back(dur.count());
        fmt::print("timing pass {} complete\n", i + 1);
        std::fflush(stdout);
      }
      fmt::print("<<<BEGIN\n");
      fmt::print(".{}", label);
      for (auto x : duration) {
        fmt::print(" {}", x);
      }
      fmt::print("\n");
      fmt::print(">>>END\n");
      std::fflush(stdout);
    } else if (bustub::StringUtil::StartsWith(opt, "explain")) {
      auto writer = bustub::SimpleStreamWriter(std::cout);
      auto x = bustub::StringUtil::Split(opt, "explain:");
      if (!x.empty() && !x[0].empty()) {
        instance.ExecuteSql(fmt::format("explain ({}) {}", x[0], sql), writer);
      } else {
        instance.ExecuteSql("explain " + sql, writer);
      }
      std::cout << std::flush;
    } else {
      throw bustub::NotImplementedException(fmt::format("unsupported extra option: {}", opt));
    }

    if (verbose) {
      fmt::print("[PASS] extra check: {}\n", opt);
      std::fflush(stdout);
    }
  }
  return true;
}

auto main(int argc, char **argv) -> int {  // NOLINT
  argparse::ArgumentParser program("bustub-sqllogictest");
  program.add_argument("file").help("the sqllogictest file to run");
  program.add_argument("--verbose").help("increase output verbosity").default_value(false).implicit_value(true);
  program.add_argument("-d", "--diff").help("write diff file").default_value(false).implicit_value(true);
  program.add_argument("--in-memory").help("use in-memory backend").default_value(false).implicit_value(true);
  program.add_argument("--bpm-size")
      .help("size of the buffer pool")
      .default_value(std::to_string(bustub::BUFFER_POOL_SIZE));
  program.add_argument("--check-min-disk-write")
      .help("the minimum disk write threshold to be checked at the end of the program");
  program.add_argument("--check-max-disk-write")
      .help("the maximum disk write threshold to be checked at the end of the program");
  program.add_argument("--check-min-disk-delete")
      .help("the maximum disk deletion threshold to be checked at the end of the program");

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  bool verbose = program.get<bool>("verbose");
  bool diff = program.get<bool>("diff");
  auto check_min_disk_write = program.present("check-min-disk-write");

  std::string filename = program.get<std::string>("file");
  std::ifstream t(filename);

  if (!t) {
    std::cerr << "Failed to open " << filename << std::endl;
    return 1;
  }

  std::string script((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  t.close();

  auto result = bustub::SQLLogicTestParser::Parse(script);
  if (result.empty()) {
    fmt::print("This is not tested this semester\n");
    return 0;
  }

  std::unique_ptr<bustub::BusTubInstance> bustub;
  size_t bpm_size = std::stoul(program.get<std::string>("bpm-size"));

  if (program.get<bool>("--in-memory")) {
    bustub = std::make_unique<bustub::BusTubInstance>(bpm_size);
  } else {
    bustub = std::make_unique<bustub::BusTubInstance>("test.bustub", bpm_size);
  }

  bustub->GenerateMockTable();

  if (bustub->buffer_pool_manager_ != nullptr) {
    bustub->GenerateTestTable();
  }

  for (const auto &record : result) {
    auto check_options = std::make_shared<bustub::CheckOptions>();
    fmt::print("{}\n", record->loc_);
    switch (record->type_) {
      case bustub::RecordType::HALT: {
        if (verbose) {
          fmt::print("{}\n", record->ToString());
        }
        return 0;
      }
      case bustub::RecordType::SLEEP: {
        if (verbose) {
          fmt::print("{}\n", record->ToString());
        }
        const auto &sleep = dynamic_cast<const bustub::SleepRecord &>(*record);
        std::this_thread::sleep_for(std::chrono::seconds(sleep.seconds_));
        continue;
      }
      case bustub::RecordType::STATEMENT: {
        const auto &statement = dynamic_cast<const bustub::StatementRecord &>(*record);
        if (verbose) {
          fmt::print("{}\n", statement.sql_);
          if (!statement.extra_options_.empty()) {
            fmt::print("Extra checks: {}\n", statement.extra_options_);
          }
        }
        try {
          if (!ProcessExtraOptions(statement.sql_, *bustub, statement.extra_options_, verbose, check_options)) {
            fmt::print("failed to process extra options\n");
            return 1;
          }

          std::stringstream result;
          auto writer = bustub::SimpleStreamWriter(result, true);
          bustub->ExecuteSql(statement.sql_, writer, check_options);
          if (verbose) {
            fmt::print("----\n{}\n", result.str());
          }
          if (statement.is_error_) {
            fmt::print("statement should error\n");
            return 1;
          }
        } catch (bustub::Exception &ex) {
          if (!statement.is_error_) {
            fmt::print("unexpected error: {}", ex.what());
            return 1;
          }
          if (verbose) {
            fmt::print("statement errored with {}", ex.what());
          }
          fflush(stdout);
        }
        continue;
      }
      case bustub::RecordType::QUERY: {
        const auto &query = dynamic_cast<const bustub::QueryRecord &>(*record);
        if (verbose) {
          fmt::print("{}\n", query.sql_);
          if (!query.extra_options_.empty()) {
            fmt::print("Extra checks: {}\n", query.extra_options_);
          }
        }
        try {
          if (!ProcessExtraOptions(query.sql_, *bustub, query.extra_options_, verbose, check_options)) {
            fmt::print("failed to process extra options\n");
            return 1;
          }

          std::stringstream result;
          auto writer = bustub::SimpleStreamWriter(result, true, " ");
          bustub->ExecuteSql(query.sql_, writer, check_options);
          if (verbose) {
            fmt::print("--- YOUR RESULT ---\n{}\n", result.str());
          }
          if (verbose) {
            fmt::print("--- EXPECTED RESULT ---\n{}\n", query.expected_result_);
          }
          if (!ResultCompare(result.str(), query.expected_result_, query.sort_mode_, diff)) {
            if (diff) {
              fmt::print("wrong result (with sort_mode={}) dumped to result.log and expected.log\n", query.sort_mode_);
            } else {
              fmt::print(
                  "wrong result (with sort_mode={}), use `-d` to store your result and expected result in a file\n",
                  query.sort_mode_);
            }
            return 1;
          }
        } catch (bustub::Exception &ex) {
          fmt::print("unexpected error: {} \n", ex.what());
          return 1;
        }
        fflush(stdout);
        continue;
      }
      default:
        throw bustub::Exception("unsupported record");
    }
  }

  if (program.is_used("--check-min-disk-write")) {
    int min_disk_write_num = std::stoi(program.get("--check-min-disk-write"));
    int actual_disk_write_num = bustub->disk_manager_->GetNumWrites();
    if (actual_disk_write_num < min_disk_write_num) {
      fmt::print("test incurred {} times of disk write, which is too low\n", actual_disk_write_num);
      return 1;
    }
  }
  if (program.is_used("--check-max-disk-write")) {
    int max_disk_write_num = std::stoi(program.get("--check-max-disk-write"));
    int actual_disk_write_num = bustub->disk_manager_->GetNumWrites();
    if (actual_disk_write_num > max_disk_write_num) {
      fmt::print("test incurred {} times of disk write, which is too high\n", actual_disk_write_num);
      return 1;
    }
  }
  if (program.is_used("--check-min-disk-delete")) {
    int min_disk_delete_num = std::stoi(program.get("--check-min-disk-delete"));
    int actual_disk_delete_num = bustub->disk_manager_->GetNumDeletes();
    if (actual_disk_delete_num < min_disk_delete_num) {
      fmt::print("test incurred {} times of disk deletion, which is too low\n", actual_disk_delete_num);
      return 1;
    }
  }

  return 0;
}
