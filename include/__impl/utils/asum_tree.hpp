#ifndef CPPARMC_ASUM_TREE_HPP
#define CPPARMC_ASUM_TREE_HPP

#include "darray.hpp"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::utils {

    template<typename ValueType=std::uint64_t, typename IndexType=std::uint64_t>
    class ASumTree {
        u_char nums_level;
        IndexType length;

        darray<ValueType> index;
        darray<ValueType> data;

        constexpr ValueType low_bit(IndexType nums);

    public:
        ASumTree(u_char nums_level);

        void add(IndexType i, ValueType val);

        inline ValueType at(IndexType i);

        inline IndexType size() const;

        inline ValueType sum();

        ValueType asum(IndexType i);

        template<typename W>
        IndexType find(W s);
    };

    template<typename ValueType, typename IndexType>
    ASumTree<ValueType, IndexType>::ASumTree(u_char nums_level):
            nums_level(nums_level),
            length(1U << nums_level),
            data(darray<ValueType>(length, 0U)),
            index(darray<ValueType>(length, 0U)) {}

    template<typename ValueType, typename IndexType>
    constexpr auto ASumTree<ValueType, IndexType>
    ::low_bit(IndexType nums) -> ValueType {
        return nums & (-nums);
    }

    template<typename ValueType, typename IndexType>
    auto ASumTree<ValueType, IndexType>
    ::at(IndexType i) -> ValueType { return data[i]; }

    template<typename ValueType, typename IndexType>
    auto ASumTree<ValueType, IndexType>
    ::size() const -> IndexType {
        return length;
    }

    template<typename ValueType, typename IndexType>
    void ASumTree<ValueType, IndexType>
    ::add(IndexType i, ValueType val) {
        i += 1U;
        data[i - 1U] += val;
        while (i <= length) {
            index[i - 1U] += val;
            i += low_bit(i);
        }
    }

    template<typename ValueType, typename IndexType>
    auto ASumTree<ValueType, IndexType>::sum() -> ValueType {
        return index[length - 1U];
    }

    template<typename ValueType, typename IndexType>
    auto ASumTree<ValueType, IndexType>::asum(IndexType i) -> ValueType {
        if (i < 0U) return 0U;
        if (i >= length) return this->sum();

        ValueType result = 0U;

        i += 1U;
        while (i > 0U) {
            result += index[i - 1U];
            i -= low_bit(i);
        }

        return result;
    }

    template<typename ValueType, typename IndexType>
    template<typename W>
    auto ASumTree<ValueType, IndexType>::find(W s) -> IndexType {
        if (s < index[0U]) return 0U;
        if (s >= index[length - 1U]) return length;

        IndexType rL = 0U, rR = length;

        while (rL + 1U != rR) {
            const IndexType rM = (rL + rR) >> 1U;
            const ValueType vM = asum(rM);
            (s >= vM ? rL : rR) = rM;
        }

        assert((asum(rL) <= s) && (s < asum(rR)));
        return rR;
    }
}

#endif //CPPARMC_ASUM_TREE_HPP
