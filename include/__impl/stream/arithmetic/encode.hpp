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

        std::uint64_t bit_buffer;
        int bit_buffer_length;

        bool clean_up_follow;
        bool follow_bit;

    public:
        CounterType input_count;

        ArithmeticEncode(Device& device, std::uint8_t symbol_bit, CounterType block_size);

        StreamStatus receive() final;
    };

    template<typename Device, typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    ArithmeticEncode<Device, SymbolType, CounterType, counter_bit>
    ::ArithmeticEncode(Device& device, std::uint8_t symbol_bit, CounterType block_size):
            InputStream<Device>(device, device.output_width, 8),
            CodecMixin<SymbolType, CounterType, counter_bit>(symbol_bit, block_size),
            ch(0),
            bit_buffer_length(0),
            bit_buffer(0),
            clean_up_follow(false),
            follow_bit(false),
            input_count(0) {}

    template<typename Device, typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    auto ArithmeticEncode<Device, SymbolType, CounterType, counter_bit>
    ::receive() -> StreamStatus {
        if (clean_up_follow) {
            if (this->follow > this->output_width) {
                this->follow -= this->output_width;
                return {this->output_width, bits::get_n_repeat_bit(follow_bit, this->output_width)};
            } else {
                clean_up_follow = false;
                const StreamStatus r = {this->follow, bits::get_n_repeat_bit(follow_bit, this->follow)};
                this->follow = 0;
                return r;
            }
        }

        ch = this->device.get();

        if (this->device.eof()) {
            if (this->follow == 0) return {-1, 0};

            this->follow += 1;
            bool output_bit = (this->L >= this->cl);
            bits::append_bit(bit_buffer, output_bit);
            bit_buffer_length += 1;

            follow_bit = !output_bit;
            clean_up_follow = true;

            const StreamStatus r = { bit_buffer_length, bit_buffer };
            bit_buffer_length = 0;
            bit_buffer = 0;
            return r;
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
                bit_buffer_length += 1;
                assert(bit_buffer_length < 64);

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

//                clean_up_follow = true;
//                follow_bit = !output_bit;

                bits::concat_bits(bit_buffer, bits::get_n_repeat_bit(!output_bit, this->follow), this->follow);
                bit_buffer_length += this->follow;
                assert(bit_buffer_length < 64);
                this->follow = 0;

            } else if ((this->cl <= this->L) && (this->R < this->cr)) {
                this->follow += 1;

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
        assert((0 <= bit_buffer) && (bit_buffer < (1U << bit_buffer_length)));
        const StreamStatus r = {bit_buffer_length, bit_buffer};
        bit_buffer_length = 0;
        bit_buffer = 0;
        return r;
    }
}

#endif //CPPARMC_ARITHMETIC_ENCODE_HPP
