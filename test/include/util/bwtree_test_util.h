#pragma once
#include "bwtree/bwtree.h"
#include "gtest/gtest.h"
#include "util/random_test_util.h"
#include "util/test_thread_pool.h"

namespace terrier {
/**
 * Normally we restrict the scope of util files to named directories/namespaces (storage, transaction, etc.) but the
 * BwTree is a unique case because it's third party code but we want to test it rigorously. This isn't a
 * third_party_test_util because directories/files with "third_party" in their name are treated differently by the CI
 * scripts.
 */

struct BwTreeTestUtil {
  BwTreeTestUtil() = delete;

  /*
   * class KeyComparator - Test whether BwTree supports context
   *                       sensitive key comparator
   *
   * If a context-sensitive KeyComparator object is being used
   * then it should follow rules like:
   *   1. There could be no default constructor
   *   2. There MUST be a copy constructor
   *   3. operator() must be const
   *
   */
  class KeyComparator {
   public:
    inline bool operator()(const int64_t k1, const int64_t k2) const { return k1 < k2; }

    explicit KeyComparator(int dummy UNUSED_ATTRIBUTE) {}

    KeyComparator() = delete;
  };

  /*
   * class KeyEqualityChecker - Tests context sensitive key equality
   *                            checker inside BwTree
   *
   * NOTE: This class is only used in KeyEqual() function, and is not
   * used as STL template argument, it is not necessary to provide
   * the object everytime a container is initialized
   */
  class KeyEqualityChecker {
   public:
    inline bool operator()(const int64_t k1, const int64_t k2) const { return k1 == k2; }

    explicit KeyEqualityChecker(int dummy UNUSED_ATTRIBUTE) {}

    KeyEqualityChecker() = delete;
  };
};

using TreeType =
    third_party::index::BwTree<int64_t, int64_t, BwTreeTestUtil::KeyComparator, BwTreeTestUtil::KeyEqualityChecker>;

}  // namespace terrier
