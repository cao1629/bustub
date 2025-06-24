//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/page/b_plus_tree_internal_page.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#pragma once

#include <queue>

#include "storage/page/b_plus_tree_page.h"

namespace bustub {

#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>
#define INTERNAL_PAGE_HEADER_SIZE 24
#define INTERNAL_PAGE_SIZE ((BUSTUB_PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(MappingType)))
/**
 *
 * Store n indexed keys and n+1 child pointers (page_id) within internal page.
 * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
 * K(i) <= K < K(i+1).
 * NOTE: since the number of keys does not equal to number of child pointers,
 * the first key always remains invalid. That is to say, any search/lookup
 * should ignore the first key.
 *
 * Internal page format (keys are stored in increasing order):
 *  --------------------------------------------------------------------------
 * | HEADER | KEY(1)+PAGE_ID(1) | KEY(2)+PAGE_ID(2) | ... | KEY(n)+PAGE_ID(n) |
 *  --------------------------------------------------------------------------
 */
INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeInternalPage : public BPlusTreePage {
 public:
  // must call initialize method after "create" a new node
  void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = INTERNAL_PAGE_SIZE);

  auto KeyAt(int index) const -> KeyType;

  void SetKeyAt(int index, const KeyType &key);

  auto ValueAt(int index) const -> ValueType;

  void SetValueAt(int index, const ValueType &value);

  // Given a value, return the index of the value in this internal page.
  auto ValueIndex(const ValueType &value) const -> int;

  // Which path to take when searching for a key
  auto Lookup(const KeyType &key, const KeyComparator &comparator) const -> ValueType;

  // InsertIntoParent -> PopulateNewRoot
  void PopulateNewRoot(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value);

  // insert a key-value pair after a key-value pair
  auto InsertNodeAfter(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value) -> int;

  void Remove(int index);

  auto RemoveAndReturnOnlyChild() -> ValueType;

  // We move all entries from this internal page to the recipient page,
  // which means this internal page is on the right of the recipient page.
  // After moving all entries, we get rid of this internal page later.
  void MoveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middle_key, BufferPoolManager *buffer_pool_manager);

  // Keep min_size entries in this internal page, and move the rest to the recipient page.
  void MoveHalfTo(BPlusTreeInternalPage *recipient, BufferPoolManager *buffer_pool_manager);

  // The first key of this internal page is now empty. After being moved to "recipient",
  // this key is not the first key of "recipient". So we set it to "middle_key".
  void MoveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                        BufferPoolManager *buffer_pool_manager);

  // The first key of "recipient" is empty. We set it to "middle_key", and then we move the last
  // entry of this internal page to the recipient page.
  void MoveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                         BufferPoolManager *buffer_pool_manager);

 private:
  // Flexible array member for page data.
  MappingType array_[1];

  // We have another internal page. We want to copy some entries from that page over here.
  // "items" is the address of the entries we want to copy from.
  void CopyNFrom(MappingType *items, int size, BufferPoolManager *buffer_pool_manager);

  // Copy one entry to the end of this internal page.
  void CopyLastFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager);

  // Copy one entry to the front of this internal page.
  void CopyFirstFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager);
};
}  // namespace bustub
