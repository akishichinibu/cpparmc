#ifndef CPPARMC_ARITHMETIC_DECODE_HPP
#define CPPARMC_ARITHMETIC_DECODE_HPP

#include <deque>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "__impl/stream/stream_base.hpp"
#include "__impl/stream/arithmetic/codec_mixin.h"
#include "__impl/utils/bit_utils.hpp"


namespace cpparmc {

    template<typename Device>
    class ArithmeticCDecode : public InputStream<Device>, public CodecMixin {
        u_int64_t uncompressed_length;
        u_int64_t output_count;
        u_int64_t value;

    public:
        ArithmeticCDecode(Device& device,
                          u_int64_t uncompressed_length,
                          const armc_params& params,
                          const armc_coder_params& coder_params);

        u_int32_t get();
    };

    template<typename Device>
    ArithmeticCDecode<Device>::ArithmeticCDecode(Device& device,
                                                 u_int64_t uncompressed_length,
                                                 const armc_params& params,
                                                 const armc_coder_params& coder_params):
            InputStream<Device>(device, 1U, 8U),
            CodecMixin(params, coder_params),
            uncompressed_length(uncompressed_length),
            output_count(0U),
            value(0U) {
        if (this->device.output_width != 1U) {
            throw std::runtime_error("Error input length. ");
        }

        for (auto i = 0; i < counter_bit; i++) {
            bool bit = this->device.get();
            if (this->device.eof()) bit = false;
            value = (value << 1U) | bit;
        }
    }

    template<typename Device>
    u_int32_t ArithmeticCDecode<Device>::get() {
        if (output_count == uncompressed_length) {
            this->_eof = true;
            return EOF;
        }
#ifdef PRINT_STAT
        for (auto i = 0; i < total_symbol; i++) {
                printf("%lu   ", accumulative_stat[i]);

                if ((i + 1) % 20 == 0) {
                    printf("\n");
                }
            }
            printf("\n");
        std::cout << std::endl;
#endif
        assert((0 <= value) && (value < counter_limit));

        const auto pos = std::upper_bound(accumulative_stat.cbegin(),
                accumulative_stat.cend(),
                static_cast<double>(value - L) / D * counter_limit);
        assert(pos != accumulative_stat.cend());

        const auto rL = pos == accumulative_stat.cbegin() ? 0 : *(pos - 1);
        const auto rR = *pos;

        u_int32_t symbol = std::distance(accumulative_stat.cbegin(), pos);
        output_count += 1U;
        stat[symbol] += 1U;
        this->update_stat();

#ifdef DECODER_DEBUG
        const auto oL = L;
        const auto oR = R;
#endif

        R = L + static_cast<double>(rR) / counter_limit * D;
        L = L + static_cast<double>(rL) / counter_limit * D;
        D = R - L;
        assert((L < R) && (R <= counter_limit));

#ifdef DECODER_DEBUG
        printf("[Output] symbol: [%u] value: [%lu] L: [%lu] R: [%lu] pos: [%lu ~ %lu]  oL [%lu]  oR [%lu]\n",
               symbol, value, L, R, *(pos - 1), *pos, oL, oR);
#endif

        while (true) {
            if (R <= cm) {
                // pass
            } else if (L >= cm) {
                assert(value >= cm);
                value -= cm;
                L -= cm;
                R -= cm;
            } else if ((L > cl) && (R <= cr)) {
                assert(value >= cl);
                value -= cl;
                L -= cl;
                R -= cl;
            } else {
                break;
            }

//            assert((L <= value) && (value < R));
#ifdef DECODER_DEBUG
            const auto ov = value;
            const auto ooL = L;
            const auto ooR = R;
#endif
            L <<= 1U;
            R <<= 1U;
            assert((L < R) && (R <= counter_limit));
            D = R - L;

            bool bit = this->device.get();
            if (this->device.eof()) bit = false;

            value = (value << 1U) | bit;
#ifdef DECODER_DEBUG
            printf("[Rerange] value: [%u] L: [%lu] R: [%lu]  ov [%lu] oL [%lu]  oR [%lu]\n", value, L, R, ov, ooL, ooR);
#endif
            assert((0 <= value) && (value < counter_limit));

        }

        return symbol;
    }
}

#endif //CPPARMC_ARITHMETIC_DECODE_HPP
