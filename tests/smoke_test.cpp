#include <gtest/gtest.h>

#include "vectorkv/version.h"

// Smoke test: confirms the test harness, the core library, and includes are
// all wired up correctly. Real module tests will replace/augment this later.
TEST(Smoke, VersionIsAvailable) {
    EXPECT_STREQ(vectorkv::version(), "0.1.0");
}
