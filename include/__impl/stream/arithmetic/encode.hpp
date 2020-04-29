#ifndef CPPARMC_ARITHMETIC_ENCODE_HPP
#define CPPARMC_ARITHMETIC_ENCODE_HPP

#include <cassert>
#include <limits>
#include <algorithm>
#include <numeric>

#include "__impl/stream/arithmetic/codec_mixin.h"
#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/bit_operation.hpp"
#include "__impl/utils/asum_tree.hpp"


namespace cpparmc::stream {

    template<typename Device,
            typename SymbolType=std::uint64_t,
            typename CounterType=std::uint64_t,
            std::uint8_t counter_bit = 56U>
    class ArithmeticEncode :
            public InputStream<Device>,
            public CodecMixin<SymbolType, CounterType, counter_bit> {

        SymbolType ch = 0;

        std::uint64_t bit_buffer = 0U;
        std::uint64_t bit_buffer_length = 0U;

    public:
        CounterType input_count;

        ArithmeticEncode(Device& device,
                         const armc_params& params,
                         const armc_coder_params& coder_params);

        std::int64_t get() final;
    };

    template<typename Device, typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    ArithmeticEncode<Device, SymbolType, CounterType, counter_bit>
    ::ArithmeticEncode(Device& device,
                       const armc_params& params,
                       const armc_coder_params& coder_params):
            InputStream<Device>(device, device.output_width, 8U),
            CodecMixin<SymbolType, CounterType, counter_bit>(params, coder_params),
            input_count(0U) {}

    template<typename Device, typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    auto ArithmeticEncode<Device, SymbolType, CounterType, counter_bit>
    ::get() -> std::int64_t {
        // pop one symbol
        while (bit_buffer_length < this->output_width) {
            ch = this->device.get();
            if (this->device.eof()) break;
            input_count += 1;
            assert((0 <= ch) && (ch < this->total_symbol));

            CounterType nR = this->model.asum(ch);
            CounterType nL = nR - this->model.at(ch);

            nR = static_cast<double>(nR) / this->model.sum() * this->counter_limit;
            nL = static_cast<double>(nL) / this->model.sum() * this->counter_limit;

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
            const auto ooL = this->L;
            const auto ooR = this->R;
#endif

            this->R = this->L + static_cast<double>(nR) / this->counter_limit * this->D;
            this->L = this->L + static_cast<double>(nL) / this->counter_limit * this->D;
            this->D = this->R - this->L;

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
            printf("[read][symbol: %5u][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][histL:%10lu ~ histR:%10lu][oL:%10lu ~ oR:%10lu]\n", ch, this->L, this->R, this->D, nL, nR, ooL, ooR);
#endif

            assert((this->L < this->R) && (this->R <= this->counter_limit));

            while (true) {
                if ((this->L >= this->cm) || (this->R < this->cm)) {
#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                    const auto oL = this->L;
                    const auto oR = this->R;
#endif
                    const bool output_bit = (this->L >= this->cm);
                    bit_buffer = bits::append_bit(bit_buffer, output_bit);
                    bit_buffer_length += 1U;

                    if (output_bit) {
                        this->R -= this->cm;
                        this->L -= this->cm;
                    }

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                        printf("[side][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][oL:%10lu ~ oR:%10lu] output:%u this->follow:%lu\n", this->L, this->R, this->D, oL, oR, output_bit, this->follow);
#endif

                    assert((this->L < this->R) && (this->R <= this->counter_limit));

                    while (this->follow > 0U) {
                        bit_buffer = bits::append_bit(bit_buffer, !output_bit);
                        bit_buffer_length += 1U;
                        this->follow -= 1U;
#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                        printf("[folw][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][oL:%10lu ~ oR:%10lu] output:%u this->follow:%lu\n", this->L, this->R, this->D, oL, oR, !output_bit, this->follow);
#endif
                    }

                } else if ((this->cl <= this->L) && (this->R < this->cr)) {
                    this->follow += 1U;

                    this->L -= this->cl;
                    this->R -= this->cl;

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                    printf("[ mid][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][this->cl:%10lu ~ this->cm:%10lu ~ this->cr:%10lu] this->follow=[%lu]\n",
                           this->L, this->R, this->D, this->cl, this->cm, this->cr, this->follow);
#endif

                    assert((this->L < this->R) && (this->R <= this->counter_limit));
                } else {
                    break;
                }

                this->R <<= 1U;
                this->L <<= 1U;
                this->D = this->R - this->L;
            }

#ifdef CPPARMC_DEBUG_PRINT_MODEL
            for (auto i = 0; i < this->total_symbol; i++) {
                printf("%lu   ", this->model.at(i));
                if ((i + 1) % 20 == 0) printf("\n");
            }
            printf("\n");
#endif

            this->update_model(ch);
        }

        if (bit_buffer_length == 0U) {
            this->_eof = true;
            return EOF;
        }

        if (this->device.eof()) {
            this->follow += 1U;
            bool output_bit = this->L >= this->cl;

            bit_buffer = bits::append_bit(bit_buffer, output_bit);
            bit_buffer_length += 1U;

            while (this->follow > 0U) {
                bit_buffer = bits::append_bit(bit_buffer, !output_bit);
                bit_buffer_length += 1U;
                this->follow -= 1U;
            }
        }

        std::uint8_t c = 0U;

        for (auto i = 0; i < this->output_width; i++) {
            if (bit_buffer_length > 0U) {
                bool bit_to_add;
                std::tie(bit_to_add, bit_buffer_length) = bits::pop_bits(bit_buffer, bit_buffer_length, 1U);
                c = bits::set_nth_bit(c, bit_to_add, this->output_width - 1U - i);
            } else {
                c = bits::set_nth_bit(c, false, this->output_width - 1U - i);
            }
        }

        return c;
    }
}

#endif //CPPARMC_ARITHMETIC_ENCODE_HPP
