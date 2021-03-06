#ifndef CPPARMC_INDEX_TREE_HPP
#define CPPARMC_INDEX_TREE_HPP

#include "darray.hpp"
#include "bit_operation.hpp"


namespace cpparmc::utils {

    template<typename ValueType=std::uint64_t, typename IndexType=std::uint64_t>
    class IndexTree {
        std::uint8_t nums_level {};
        IndexType length;

        darray <ValueType> index;
        darray <ValueType> data;

        constexpr ValueType low_bit(IndexType nums) noexcept;

    public:
        IndexTree() noexcept = default;

        explicit IndexTree(std::uint8_t nums_level) noexcept;

        void add(IndexType i, ValueType val) noexcept;

        inline ValueType at(IndexType i) noexcept;

        inline IndexType size() const noexcept;

        inline ValueType sum() noexcept;

        ValueType accumulate_sum(IndexType i) noexcept;

        template<typename W>
        IndexType find(W s) noexcept;
    };

    template<typename V, typename T>
    IndexTree<V, T>::IndexTree(std::uint8_t nums_level) noexcept :
            nums_level(nums_level),
            length(1U << nums_level),
            data(darray<V>(length, 0)),
            index(darray<V>(length, 0)) {}

    template<typename ValueType, typename IndexType>
    constexpr auto IndexTree<ValueType, IndexType>
    ::low_bit(IndexType nums) noexcept -> ValueType {
        return nums & (-nums);
    }

    template<typename ValueType, typename IndexType>
    auto IndexTree<ValueType, IndexType>
    ::at(IndexType i) noexcept -> ValueType {
        return data[i];
    }

    template<typename ValueType, typename IndexType>
    auto IndexTree<ValueType, IndexType>
    ::size() const noexcept -> IndexType {
        return length;
    }

    template<typename ValueType, typename IndexType>
    void IndexTree<ValueType, IndexType>
    ::add(IndexType i, ValueType val) noexcept {
        i += 1;
        data[i - 1] += val;
        while (i <= length) {
            index[i - 1] += val;
            i += low_bit(i);
        }
    }

    template<typename ValueType, typename IndexType>
    auto IndexTree<ValueType, IndexType>::sum() noexcept -> ValueType {
        return index[length - 1];
    }

    template<typename ValueType, typename IndexType>
    auto IndexTree<ValueType, IndexType>::accumulate_sum(IndexType i) noexcept -> ValueType {
        if (i < 0) return 0;
        if (i >= length) return this->sum();

        ValueType result = 0;

        i += 1;
        while (i > 0) {
            result += index[i - 1];
            i -= low_bit(i);
        }

        return result;
    }

    template<typename ValueType, typename IndexType>
    template<typename W>
    auto IndexTree<ValueType, IndexType>::find(W s) noexcept -> IndexType {
        if (s < index[0]) return 0;
        if (s >= index[length - 1]) return length;

        IndexType rL = 0, rR = length;

        while (rL + 1 != rR) {
            const IndexType rM = (rL + rR) >> 1U;
            const ValueType vM = accumulate_sum(rM);
            (s >= vM ? rL : rR) = rM;
        }

        assert((accumulate_sum(rL) <= s) && (s < accumulate_sum(rR)));
        return rR;
    }
}

#endif //CPPARMC_INDEX_TREE_HPP
