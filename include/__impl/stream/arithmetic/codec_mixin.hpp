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

        armc_params params{};
        armc_coder_params coder_params{};

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
        inline CodecMixin(const armc_params& params, const armc_coder_params& coder_params);

        inline void update_model(SymbolType symbol);
    };

    template<typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    CodecMixin<SymbolType, CounterType, counter_bit>
    ::CodecMixin(const armc_params& params, const armc_coder_params& coder_params):
            params(params),
            coder_params(coder_params),
            total_symbol(1U << params.symbol_bit),
            L(0U), R(counter_limit), D(R - L), follow(0U),
            model(params.symbol_bit) {
        for (auto i = 0; i < model.size(); i++) model.add(i, 1U);
    }

    template<typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    void CodecMixin<SymbolType, CounterType, counter_bit>
    ::update_model(SymbolType symbol) {
        model.add(symbol, 1U);
    }
}

#endif //CPPARMC_ARITHMETIC_CODEC_MIXIN_H
