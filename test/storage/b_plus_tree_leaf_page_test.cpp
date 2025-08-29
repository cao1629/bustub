#include "storage/page/b_plus_tree_leaf_page.h"

#include <gtest/gtest.h>

#include "common/config.h"
#include "test_util.h"

namespace bustub {

TEST(BPlusTreeLeadPageTest, SimpleTest) {
  char page1[200];
  memset(page1, 0, 200);
  char page2[200];
  memset(page2, 0, 200);

  auto leaf1 = reinterpret_cast<BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>*>(page1);
  leaf1->Init(1);
  auto leaf2 = reinterpret_cast<BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>*>(page2);
  leaf2->Init(2);

  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  GenericKey<8> index_key1;
  RID rid1{10, 10};
  index_key1.SetFromInteger(10);
  leaf1->Insert(index_key1, rid1, comparator);

  GenericKey<8> index_key2;
  RID rid2{10, 20};
  index_key2.SetFromInteger(20);
  leaf1->Insert(index_key2, rid2, comparator);

  GenericKey<8> index_key3;
  RID rid3{10, 30};
  index_key3.SetFromInteger(30);
  leaf2->Insert(index_key3, rid3, comparator);

  leaf1->MoveFirstToEndOf(leaf2);

}

}