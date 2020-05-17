#ifndef CPPARMC_ARITHMETIC_DECODE_HPP
#define CPPARMC_ARITHMETIC_DECODE_HPP

#include <iostream>
#include <algorithm>
#include <numeric>

#include "stream/arithmetic/mixin.hpp"
#include "utils/bit_operation.hpp"


namespace cpparmc::stream {

    template<typename Source,
            typename CounterType=std::uint64_t,
            StreamSizeType counter_bit = 52>
    class ArithmeticDecode:
            public Generator<Source>,
            protected ArithmeticCodecMixin<CounterType, counter_bit> {
        CounterType value;

    public:
        inline explicit ArithmeticDecode(Source& src) noexcept;

        inline StreamStatus patch() noexcept final;
    };

    template<typename Source, typename CounterType, StreamSizeType cb>
    ArithmeticDecode<Source, CounterType, cb>::ArithmeticDecode(Source& src) noexcept:
            Generator<Source>(src),
            ArithmeticCodecMixin<CounterType, cb>([&]() {
                const auto frame = src.next(8);
                return std::get<1>(frame.value());
            }()),
            value([&]() {
                return std::get<1>(src.next(cb, true).value());
            }()) {
#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
        DEBUG_PRINT("The symbol bit of arithmetic code is {:d}. value {:d}. ",
                this->symbol_bit,
                value);
#endif
        assert((this->L <= value) && (value < this->R));
        CounterType rR = this->model
                .template find<CounterType>(static_cast<double>(value - this->L) / this->D * this->model.sum());
        assert(rR != this->model.size());

        SymbolType symbol = rR;
        this->send(this->symbol_bit, symbol);
    }

    template<typename Device, typename CounterType, StreamSizeType cb>
    auto ArithmeticDecode<Device, CounterType, cb>
    ::patch() noexcept -> StreamStatus {
        if (this->src_eof()) return std::nullopt;
        assert((this->L <= value) && (value < this->R));

        CounterType rR = this->model
                .template find<CounterType>(static_cast<double>(value - this->L) / this->D * this->model.sum());
        assert(rR != this->model.size());

        SymbolType symbol = rR;
        this->send(this->symbol_bit, symbol);

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
        const auto oL = this->L;
        const auto oR = this->R;
        const auto oD = this->D;
        const auto target = static_cast<double>(value - this->L) / this->D * this->model.sum();
        const auto oSum = this->model.sum();
#endif

        CounterType vL = this->model.accumulate_sum(rR - 1U);
        CounterType vR = this->model.accumulate_sum(rR);

        this->R = this->L + static_cast<double>(vR) / this->model.sum() * this->D;
        this->L = this->L + static_cast<double>(vL) / this->model.sum() * this->D;
        this->D = this->R - this->L;
        this->update_model(symbol);

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
        DEBUG_PRINT("[Output] symbol:[{:10d}] value:[{:10d}][L={:10d} ~ R={:10d}] "
                     "pos=[*** ~ {:5d}] "
                     "[oL={:10d} ~ oR={:10d}]",
                     symbol, value, this->L, this->R, rR, oL, oR);
#endif
        assert((this->L < this->R) && (this->R <= this->counter_limit));


        while (true) {
            assert((this->L <= value) && (value < this->R));

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            const auto ov = value;
#endif

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            const auto oov = value;
            const auto ooL = this->L;
            const auto ooR = this->R;
#endif
            if (this->R <= this->cm) {
                // pass
            } else if (this->L >= this->cm) {
                value -= this->cm;
                this->L -= this->cm;
                this->R -= this->cm;
            } else if ((this->L > this->cl) && (this->R <= this->cr)) {
                value -= this->cl;
                this->L -= this->cl;
                this->R -= this->cl;
            } else {
                break;
            }
#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            const auto ooL2 = this->L;
            const auto ooR2 = this->R;
#endif
            this->L <<= 1U;
            this->R <<= 1U;

            assert((this->L < this->R) && (this->R <= this->counter_limit));
            this->D = this->R - this->L;

            bool bit = this->src.next();

            if (this->src_eof()) {
                bit = false;
            }

            bits::append_bit(value, bit);

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            DEBUG_PRINT("[Rerange] "
                        "value=[{:d}][L={:d} ~ R={:d}] ov=[{:d}][oL={:d} ~ oR={:d}]",
                        value, this->L, this->R, ov, oov, ooL, ooR);
#endif

            assert((this->L <= value) && (value < this->R));
        }

        return empty_frame;
    }
}

#endif //CPPARMC_ARITHMETIC_DECODE_HPP
