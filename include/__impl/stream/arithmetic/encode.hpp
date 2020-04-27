#ifndef CPPARMC_ARITHMETIC_ENCODE_HPP
#define CPPARMC_ARITHMETIC_ENCODE_HPP

#include <cassert>
#include <deque>
#include <limits>
#include <algorithm>
#include <numeric>

#include "__impl/stream/arithmetic/codec_mixin.h"
#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/bit_operation.hpp"
#include "__impl/utils/asum_tree.hpp"


namespace cpparmc::stream {

    template<typename Device>
    class ArithmeticEncode : public InputStream<Device>, public CodecMixin {
        u_int64_t ch = 0;
        std::deque<bool> bit_buffer;

    public:
        u_int64_t input_count;

        ArithmeticEncode(Device& device,
                         const armc_params& params,
                         const armc_coder_params& coder_params);

        u_int64_t get() final;
    };

    template<typename Device>
    ArithmeticEncode<Device>::ArithmeticEncode(Device& device,
                                               const armc_params& params,
                                               const armc_coder_params& coder_params):
            InputStream<Device>(device, device.output_width, 8U),
            CodecMixin(params, coder_params),
            input_count(0U) {}

    template<typename Device>
    u_int64_t ArithmeticEncode<Device>::get() {
        // pop one symbol
        while (bit_buffer.size() < this->output_width) {
            ch = this->device.get();
            if (this->device.eof()) break;
            input_count += 1;
            assert((0 <= ch) && (ch < total_symbol));

            const auto nL = ch == 0 ? 0 : accumulative_stat[ch - 1];
            const auto nR = accumulative_stat[ch];

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
            const auto ooL = L;
            const auto ooR = R;
#endif

            R = L + static_cast<double>(nR) / counter_limit * D;
            L = L + static_cast<double>(nL) / counter_limit * D;
            D = R - L;

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
            printf("[read][symbol: %5u][L:%10lu ~ R:%10lu ~ D:%10lu][histL:%10lu ~ histR:%10lu][oL:%10lu ~ oR:%10lu]\n", ch, L, R, D, nL, nR, ooL, ooR);
#endif

            assert((L < R) && (R <= this->counter_limit));

            while (true) {
                if ((L >= cm) || (R < cm)) {
#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                    const auto oL = L;
                    const auto oR = R;
#endif
                    const bool output_bit = (L >= cm);
                    bit_buffer.push_back(output_bit);

                    if (output_bit) {
                        R -= cm;
                        L -= cm;
                    }

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                        printf("[side][L:%10lu ~ R:%10lu ~ D:%10lu][oL:%10lu ~ oR:%10lu] output:%u follow:%lu\n", L, R, D, oL, oR, output_bit, follow);
#endif

                    assert((L < R) && (R <= counter_limit));

                    while (follow > 0) {
                        bit_buffer.push_back(!output_bit);
                        follow -= 1;
#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                        printf("[folw][L:%10lu ~ R:%10lu ~ D:%10lu][oL:%10lu ~ oR:%10lu] output:%u follow:%lu\n", L, R, D, oL, oR, !output_bit, follow);
#endif
                    }

                } else if ((cl <= L) && (R < cr)) {
                    follow += 1;

                    L -= cl;
                    R -= cl;

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                    printf("[ mid][L:%10lu ~ R:%10lu ~ D:%10lu][cl:%10lu ~ cm:%10lu ~ cr:%10lu] follow=[%lu]\n",
                           L, R, D, cl, cm, cr, follow);
#endif

                    assert((L < R) && (R <= counter_limit));
                } else {
                    break;
                }

                R <<= 1U;
                L <<= 1U;
                D = R - L;
            }

            this->update_model(ch);

#ifdef CPPARMC_DEBUG_PRINT_MODEL
            for (auto i = 0; i < total_symbol; i++) {
                printf("%lu   ", accumulative_stat[i]);
                if ((i + 1) % 20 == 0) printf("\n");
            }
#endif
        }

        if (bit_buffer.empty()) {
            this->_eof = true;
            return EOF;
        }

        if (this->device.eof()) {
            follow += 1;
            bool output_bit = L >= cl;
            bit_buffer.push_back(output_bit);
            while (follow > 0) {
                bit_buffer.push_back(!output_bit);
                follow -= 1;
            }
        }

        u_char c = 0U;

        for (auto i = 0; i < this->output_width; i++) {
            c = bits::set_nth_bit(c, bit_buffer.empty() ? false : bit_buffer.front(), this->output_width - 1 - i);
            if (!bit_buffer.empty()) bit_buffer.pop_front();
        }

        return c;
    }
}

#endif //CPPARMC_ARITHMETIC_ENCODE_HPP
