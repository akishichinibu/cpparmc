#ifndef CPPARMC_ARITHMETIC_CODEC_MIXIN_H
#define CPPARMC_ARITHMETIC_CODEC_MIXIN_H

#include "setting.h"
#include "__impl/darray.hpp"


namespace cpparmc {
    class CodecMixin {

    public:
        constexpr static u_int8_t counter_bit = 62U;
        constexpr static u_int64_t counter_limit = 1LU << counter_bit;

        armc_params params{};
        armc_coder_params coder_params{};

        u_int64_t total_symbol;
        u_int64_t L, R, D, follow;

        darray<u_int64_t> stat;
        darray<u_int64_t> accumulative_stat;

    protected:
        // [0, 0.25)
        constexpr static u_int64_t cl = counter_limit >> 2U;
        // 0.5
        constexpr static u_int64_t cm = counter_limit >> 1U;
        // [0.75, 1)
        constexpr static u_int64_t cr = cl + cm;

    public:
        CodecMixin(const armc_params& params, const armc_coder_params& coder_params):
                   params(params),
                   coder_params(coder_params),
                   total_symbol(1U << params.symbol_bit),
                   L(0U), R(counter_limit), D(R - L), follow(0U),
                   stat(total_symbol, 1),
                   accumulative_stat(total_symbol) {
            this->update_stat();
        };

        void update_stat() {
            const auto stat_sum = std::accumulate(stat.cbegin(), stat.cend(), 0UL);

            std::partial_sum(stat.cbegin(), stat.cend(), accumulative_stat.begin());

            std::transform(accumulative_stat.cbegin(),
                           accumulative_stat.cend(),
                           accumulative_stat.begin(),
                           [=](auto r) { return static_cast<double>(r) / stat_sum * counter_limit; });
        }
    };
}


#endif //CPPARMC_ARITHMETIC_CODEC_MIXIN_H
