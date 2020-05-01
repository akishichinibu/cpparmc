#ifndef CPPARMC_ARITHMETIC_ENCODE_HPP
#define CPPARMC_ARITHMETIC_ENCODE_HPP

#include <cassert>
#include <limits>
#include <algorithm>
#include <numeric>

#include "__impl/stream/arithmetic/codec_mixin.hpp"
#include "__impl/stream/stream_base.hpp"

#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::stream {

    using namespace setting;

    template<typename Device,
            typename SymbolType=std::uint64_t,
            typename CounterType=std::uint64_t,
            std::uint8_t counter_bit = default_count_bit>
    class ArithmeticEncode:
            public InputStream<Device>,
            public CodecMixin<SymbolType, CounterType, counter_bit> {

        SymbolType ch = 0;

        std::uint64_t bit_buffer = 0U;
        std::uint8_t bit_buffer_length = 0U;

    public:
        CounterType input_count;

        ArithmeticEncode(Device& device,
                         const armc_params& params,
                         const armc_coder_params& coder_params);

        std::pair<std::uint8_t, std::uint64_t> receive() final;
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
    ::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        ch = this->device.get();

        if (this->device.eof()) {
            this->follow += 1U;
            bool output_bit = (this->L >= this->cl);

            bits::append_bit(bit_buffer, output_bit);
            bit_buffer_length += 1U;

            while (this->follow > 0U) {
                bits::append_bit(bit_buffer, !output_bit);
                bit_buffer_length += 1U;
                this->follow -= 1U;
            }

            this->_eof = true;
            return { bit_buffer_length, bit_buffer };
        }

        assert((0 <= ch) && (ch < this->total_symbol));
        input_count += 1;

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
        printf("[read][symbol: %5u][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][histL:%10lu ~ histR:%10lu][oL:%10lu ~ oR:%10lu]\n",
                ch, this->L, this->R, this->D, nL, nR, ooL, ooR);
#endif

        assert((this->L < this->R) && (this->R <= this->counter_limit));

        while (true) {
            if ((this->L >= this->cm) || (this->R < this->cm)) {
#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                const auto oL = this->L;
                    const auto oR = this->R;
#endif
                const bool output_bit = (this->L >= this->cm);
                bits::append_bit(bit_buffer, output_bit);
                bit_buffer_length += 1U;

                if (output_bit) {
                    this->R -= this->cm;
                    this->L -= this->cm;
                }

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                printf("[side][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][oL:%10lu ~ oR:%10lu] output:%u this->follow:%lu\n", this->L, this->R, this->D, oL, oR, output_bit, this->follow);
#endif

                assert((this->L < this->R) && (this->R <= this->counter_limit));

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                printf("[folw][this->L:%10lu ~ this->R:%10lu ~ this->D:%10lu][oL:%10lu ~ oR:%10lu] output:%u this->follow:%lu\n",
                        this->L, this->R, this->D, oL, oR, !output_bit, this->follow);
#endif

                bits::concat_bits(bit_buffer, bits::get_n_repeat_bit(!output_bit, this->follow), this->follow);
                bit_buffer_length += this->follow;
                this->follow = 0U;

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
        const auto temp = bit_buffer_length;
        bit_buffer_length = 0;
        return { temp, bit_buffer };

    }
}

#endif //CPPARMC_ARITHMETIC_ENCODE_HPP
