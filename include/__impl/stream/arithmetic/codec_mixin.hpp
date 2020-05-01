#ifndef CPPARMC_ARITHMETIC_CODEC_MIXIN_H
#define CPPARMC_ARITHMETIC_CODEC_MIXIN_H

#include "setting.h"
#include "__impl/utils/darray.hpp"
#include "__impl/utils/asum_tree.hpp"


namespace cpparmc {

    using namespace setting;

    template<typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    class CodecMixin {

    protected:
        static_assert(counter_bit <= std::numeric_limits<CounterType>::digits);
        constexpr static CounterType counter_limit = 1LU << counter_bit;

        std::uint8_t symbol_bit;
        CounterType block_size;

        CounterType total_symbol;
        CounterType L, R, D, follow;

        utils::ASumTree<CounterType, SymbolType> model;

        // [0, 0.25)
        constexpr static CounterType cl = counter_limit >> 2U;
        // 0.5
        constexpr static CounterType cm = counter_limit >> 1U;
        // [0.75, 1)
        constexpr static CounterType cr = cl + cm;

    public:
        inline CodecMixin(std::uint8_t symbol_bit, CounterType block_size=0);

        inline void update_model(SymbolType symbol);
    };

    template<typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    CodecMixin<SymbolType, CounterType, counter_bit>
    ::CodecMixin(std::uint8_t symbol_bit, CounterType block_size):
            symbol_bit(symbol_bit),
            block_size(block_size),
            total_symbol(1U << symbol_bit),
            L(0), R(counter_limit), D(R - L), follow(0),
            model(symbol_bit) {
        for (auto i = 0; i < model.size(); i++) model.add(i, 1);
    }

    template<typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    void CodecMixin<SymbolType, CounterType, counter_bit>
    ::update_model(SymbolType symbol) {
        model.add(symbol, 1);
    }
}

#endif //CPPARMC_ARITHMETIC_CODEC_MIXIN_H
