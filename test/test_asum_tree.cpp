#include <gtest/gtest.h>
#include <random>

#include "utils/darray.hpp"
#include "utils/index_tree.hpp"

namespace cu = cpparmc::utils;

TEST(TEST_CPPARMC, test_asum_tree_simple) {
    const auto H = 3U;
    auto test_tree = cu::IndexTree(H);

    test_tree.add(0, 1);
    test_tree.add(1, 1);
    test_tree.add(4, 5);
    test_tree.add(5, 2);
    test_tree.add(7, 6);

    ASSERT_EQ(test_tree.find(-1), 8);
    ASSERT_EQ(test_tree.find(3), 4);
    ASSERT_EQ(test_tree.find(5), 4);
    ASSERT_EQ(test_tree.find(8), 5);
    ASSERT_EQ(test_tree.find(12), 7);
    ASSERT_EQ(test_tree.find(13), 7);
    ASSERT_EQ(test_tree.find(1000), 8);
}

TEST(TEST_CPPARMC, test_asum_tree_compare) {

    for (auto H = 1U; H <= 16U; H++) {
        const auto nums = 1U << H;
        const auto N = 10000;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis_t(0, nums);
        std::uniform_int_distribution<> dis_v(0, 100);

        auto test_buf = cu::darray<std::uint32_t>(nums, 0);
        auto test_accu = cu::darray<std::uint32_t>(nums, 0);

        auto test_tree = cu::IndexTree<std::uint32_t, std::uint32_t>(H);

        for (auto i = 0; i < N; i++) {
            const auto t = dis_t(gen);
            const auto v = dis_v(gen);

            test_buf[t] += v;
            test_tree.add(t, v);

            const auto r = dis_t(gen);

            const auto output1 = test_tree.find(r);

            const auto stat_sum = std::accumulate(test_buf.cbegin(), test_buf.cend(), 0UL);

            std::partial_sum(test_buf.cbegin(), test_buf.cend(), test_accu.begin());

            const auto pos = std::upper_bound(test_accu.cbegin(), test_accu.cend(), r);

            const auto output2 = std::distance(test_accu.cbegin(), pos);

            ASSERT_EQ(output1, output2);
        }
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
