#ifndef CPPARMC_ASUM_TREE_HPP
#define CPPARMC_ASUM_TREE_HPP

#include "__impl/darray.hpp"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::utils {

    template<typename T, typename R>
    class ASumTree {
        typedef T value_type;
        typedef R index_type;

        u_char level;
        darray<value_type> data;

    public:
        ASumTree(u_char level);

        value_type add(index_type index, value_type val, bool return_result=false);

        value_type asum(index_type index);

        value_type find(value_type s);
    };

    template<typename T, typename R>
    ASumTree<T, R>::ASumTree(u_char level):
    level(level), data(darray<value_type>((1U << level) - 1U), 0U) {}

    template<typename T, typename R>
    auto ASumTree<T, R>::asum(index_type index) -> value_type {
        index += 1U;
        if (index == 1U) return data[index - 1U];

        const auto height = bits::get_bits_width(index);
        value_type result = 0U;

        for (auto i = height - 1U; i >= 0U; i--) {
            const value_type cur = index >> i;
            const bool choice = bits::get_nth_bit(index, i);

            if (choice) {
                result += data[((cur << 1U) | 0U) - 1U];
            }

            cur = (cur << 1U) | choice;
        }

        return result + data[index - 1U];
    }

    template<typename T, typename R>
    auto ASumTree<T, R>::add(index_type index, value_type val, bool return_result) -> value_type {
        const auto shift_index = index + 1U;
        const auto height = bits::get_bits_width(shift_index);

        for (auto i = height - 1U; i >= 0U; i--) {
            const value_type cur = index >> i;
            data[cur - 1U] += val;
        }

        return return_result ? asum(index) - asum(index - 1U) : 0U;
    }


}

#endif //CPPARMC_ASUM_TREE_HPP
