#include <gtest/gtest.h>
#include <vector>

#include "vectorkv/distance.h"

TEST(Distance, IdenticalVectorsAreOne) {
    std::vector<float> v = {1.0f, 2.0f, 3.0f};
    EXPECT_NEAR(vectorkv::cosine_similarity(v, v), 1.0f, 1e-6);
}

TEST(Distance, OrthogonalVectorsAreZero) {
    std::vector<float> a = {1.0f, 0.0f};
    std::vector<float> b = {0.0f, 1.0f};
    EXPECT_NEAR(vectorkv::cosine_similarity(a, b), 0.0f, 1e-6);
}

TEST(Distance, ZeroVectorReturnsZero) {
    std::vector<float> a = {0.0f, 0.0f};
    std::vector<float> b = {1.0f, 1.0f};
    EXPECT_NEAR(vectorkv::cosine_similarity(a, b), 0.0f, 1e-6);
}