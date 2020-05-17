#ifndef CPPARMC_ARITHMETIC_CODEC_MIXIN_H
#define CPPARMC_ARITHMETIC_CODEC_MIXIN_H

#include "utils/darray.hpp"
#include "utils/index_tree.hpp"
#include "stream/generator.hpp"


namespace cpparmc::stream {

    template<typename CounterType, StreamSizeType cb>
    class ArithmeticCodecMixin {
        static_assert(cb <= std::numeric_limits<CounterType>::digits);

    protected:
        constexpr static CounterType counter_limit = 1LU << cb;
        // [0, 0.25)
        constexpr static CounterType cl = counter_limit >> 2LU;
        // 0.5
        constexpr static CounterType cm = counter_limit >> 1LU;
        // [0.75, 1)
        constexpr static CounterType cr = cl + cm;

        std::uint8_t symbol_bit {};
        CounterType total_symbol;

        CounterType L, R, D, follow;
        utils::IndexTree<CounterType, SymbolType> model;

    public:
        inline explicit ArithmeticCodecMixin(StreamSizeType symbol_bit) noexcept;

        inline ArithmeticCodecMixin() = default;

        inline void update_model(SymbolType symbol) noexcept;
    };

    template<typename CounterType, StreamSizeType cb>
    ArithmeticCodecMixin<CounterType, cb>
    ::ArithmeticCodecMixin(StreamSizeType symbol_bit) noexcept:
            symbol_bit(symbol_bit),
            total_symbol(1LU << symbol_bit),
            L(0UL), R(counter_limit), D(R - L), follow(0UL),
            model(symbol_bit) {
        for (auto i = 0; i < model.size(); i++) model.add(i, 1);
    }

    template<typename CounterType, StreamSizeType cb>
    void ArithmeticCodecMixin<CounterType, cb>
    ::update_model(SymbolType symbol) noexcept {
        model.add(symbol, 1);
    }
}

#endif //CPPARMC_ARITHMETIC_CODEC_MIXIN_H
