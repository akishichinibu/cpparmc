#ifndef CPPARMC_ARITHMETIC_DECODE_HPP
#define CPPARMC_ARITHMETIC_DECODE_HPP

#include <deque>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "__impl/stream/stream_base.hpp"
#include "__impl/stream/arithmetic/codec_mixin.h"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc {
    namespace stream {

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

            u_int64_t get();
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
                value = bits::append_bit(value, bit);
            }
        }

        template<typename Device>
        u_int64_t ArithmeticCDecode<Device>::get() {
            if (output_count == uncompressed_length) {
                this->_eof = true;
                return EOF;
            }

#ifdef CPPARMC_DEBUG_PRINT_MODEL
                for (auto i = 0; i < total_symbol; i++) {
                    printf("%lu   ", accumulative_stat[i]);
                    if ((i + 1) % 20 == 0) printf("\n");
                }
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
            this->update_model(symbol);

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            const auto oL = L;
            const auto oR = R;
#endif

            R = L + static_cast<double>(rR) / counter_limit * D;
            L = L + static_cast<double>(rL) / counter_limit * D;
            D = R - L;
            assert((L < R) && (R <= counter_limit));

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            spdlog::debug("[Output] symbol: {:10d} value: {:10d} [L:{:10d} ~ R:{:10d}] pos: [{:5d} ~ {:5d}] [oL:{:10d} ~ oR:{:10d}]",
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

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
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
                value = bits::append_bit(value, bit);
#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
                spdlog::debug("[Rerange] value: {:d} L: {:d} R: {:d}  ov {:d} oL {:d} oR {:d}",
                        value, L, R, ov, ooL, ooR);
#endif

                assert((0 <= value) && (value < counter_limit));
            }

            return symbol;
        }
    }
}

#endif //CPPARMC_ARITHMETIC_DECODE_HPP
