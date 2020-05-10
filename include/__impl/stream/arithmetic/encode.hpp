#ifndef CPPARMC_ARITHMETIC_ENCODE_HPP
#define CPPARMC_ARITHMETIC_ENCODE_HPP

#include <cassert>
#include <limits>
#include <algorithm>
#include <numeric>

#include "__impl/utils/bit_operation.hpp"
#include "__impl/stream/arithmetic/mixin.hpp"
#include "__impl/stream/generator.hpp"


namespace cpparmc::stream {

    using namespace setting;

    typedef struct {
    } VoidSource;

    template<typename Source=VoidSource,
            typename CounterType=std::uint64_t,
            StreamSizeType counter_bit = default_count_bit,
            std::size_t init_bit_buffer_size = 256>
    class ArithmeticEncode:
            public Generator<Source>,
            protected ArithmeticCodecMixin<CounterType, counter_bit> {

        static_assert((1LU << (counter_bit + 1U)) - 1U <= std::numeric_limits<CounterType>::max());
        SymbolType ch;
        std::vector<bool> bit_buffer;
        std::size_t buffer_head;

    public:
        ArithmeticEncode(Source& src, StreamSizeType symbol_bit) noexcept;

        StreamStatus patch() noexcept final;
    };

    template<typename Source,
            typename CounterType,
            StreamSizeType cb,
            std::size_t ib>
    ArithmeticEncode<Source, CounterType, cb, ib>
    ::ArithmeticEncode(Source& src, StreamSizeType symbol_bit) noexcept:
            Generator<Source>(src),
            ArithmeticCodecMixin<CounterType, cb>(symbol_bit),
            ch(0),
            buffer_head(0) {
        bit_buffer.reserve(ib);
        this->send(std::numeric_limits<StreamSizeType>::digits, this->symbol_bit);
    }

    template<typename Source,
            typename CounterType,
            StreamSizeType cb,
            std::size_t ib>
    auto ArithmeticEncode<Source, CounterType, cb, ib>
    ::patch() noexcept -> StreamStatus {
        if (buffer_head != bit_buffer.size()) {
            return StreamStatus(std::in_place, 1, bit_buffer.at(buffer_head++));
        }

        bit_buffer.resize(0);
        buffer_head = 0;

        const auto frame = this->src.next(this->symbol_bit);

        if (this->src.eof()) {
            if (this->follow == 0) return std::nullopt;

            this->follow += 1;
            bool output_bit = (this->L >= this->cl);
            bit_buffer.push_back(output_bit);

            bool follow_bit = !output_bit;
            const auto origin_tail = bit_buffer.size();
            bit_buffer.resize(origin_tail + this->follow);
            for (auto i = 0; i < this->follow; i++) bit_buffer[origin_tail + i] = follow_bit;

            this->follow = 0;
            return StreamStatus(std::in_place, 0, 0);
        }

        ch = std::get<1>(frame.value());
        assert((0 <= ch) && (ch < this->total_symbol));

        CounterType nR = this->model.accumulate_sum(ch);
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
        DEBUG_PRINT("[read]"
                    "[symbol: {:d}]"
                    "[this->L:{:d} ~ this->R:{:d} ~ this->D:{:d}]"
                    "[histL:{:d} ~ histR:{:d}][oL:{:d} ~ oR:{:d}]",
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
                bit_buffer.push_back(output_bit);

                if (output_bit) {
                    this->R -= this->cm;
                    this->L -= this->cm;
                }

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                    DEBUG_PRINT("[side]"
                                "[this->L:{:d} ~ this->R:{:d} ~ this->D:{:d}]"
                                "[oL:{:d} ~ oR:{:d}] "
                                "output:{:d} this->follow:{:d}",
                                this->L, this->R, this->D, oL, oR, output_bit, this->follow);
#endif

                assert((this->L < this->R) && (this->R <= this->counter_limit));

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                DEBUG_PRINT("[folw]"
                            "[this->L:{:d} ~ this->R:{:d} ~ this->D:{:d}]"
                            "[oL:{:d} ~ oR:{:d}] "
                            "output:{:d} this->follow:{:d}",
                            this->L, this->R, this->D, oL, oR, !output_bit, this->follow);
#endif
                bool follow_bit = !output_bit;
                const auto origin_tail = bit_buffer.size();
                bit_buffer.resize(origin_tail + this->follow);
                for (; this->follow > 0; this->follow--) {
                    bit_buffer[origin_tail + this->follow - 1] = follow_bit;
                }
            } else if ((this->cl <= this->L) && (this->R < this->cr)) {
                this->follow += 1;

                this->L -= this->cl;
                this->R -= this->cl;

#ifdef CPPARMC_DEBUG_ARITHMETIC_ENCODER
                DEBUG_PRINT("[ mid]"
                            "[this->L:{:d} ~ this->R:{:d} ~ this->D:{:d}]"
                            "[this->cl:{:d} ~ this->cm:{:d} ~ this->cr:{:d}] this->follow=[{:d}]",
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
        return StreamStatus(std::in_place, 0, 0);
    }
}

#endif //CPPARMC_ARITHMETIC_ENCODE_HPP
