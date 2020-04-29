#ifndef CPPARMC_ARITHMETIC_DECODE_HPP
#define CPPARMC_ARITHMETIC_DECODE_HPP

#include <deque>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "__impl/stream/stream_base.hpp"
#include "__impl/stream/arithmetic/codec_mixin.h"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::stream {

    template<typename Device,
            typename SymbolType=std::int64_t,
            typename CounterType=std::int64_t,
            std::uint8_t counter_bit = 52U>
    class ArithmeticDecode :
            public InputStream<Device>,
            public CodecMixin<SymbolType, CounterType, counter_bit> {

        std::uint64_t uncompressed_length;
        std::uint64_t output_count;
        CounterType value;

    public:
        ArithmeticDecode(Device& device,
                         std::uint64_t uncompressed_length,
                         const armc_params& params,
                         const armc_coder_params& coder_params);

        std::int64_t get();
    };

    template<typename Device, typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    ArithmeticDecode<Device, SymbolType, CounterType, counter_bit>
    ::ArithmeticDecode(Device& device,
                       std::uint64_t uncompressed_length,
                       const armc_params& params,
                       const armc_coder_params& coder_params):
            InputStream<Device>(device, 1U, 8U),
            CodecMixin<SymbolType, CounterType, counter_bit>(params, coder_params),
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

    template<typename Device, typename SymbolType, typename CounterType, std::uint8_t counter_bit>
    auto ArithmeticDecode<Device, SymbolType, CounterType, counter_bit>
    ::get() -> std::int64_t {
        if (output_count == uncompressed_length) {
            this->_eof = true;
            return EOF;
        }

#ifdef CPPARMC_DEBUG_PRINT_MODEL
            for (auto i = 0; i < this->total_symbol; i++) {
                printf("%lu   ", this->model.at(i));
                if ((i + 1) % 20 == 0) printf("\n");
            }
            printf("\n");
#endif

        assert((this->L <= value) && (value < this->R));

        CounterType rR = this->model
                .template find<CounterType>(static_cast<double>(value - this->L) / this->D * this->model.sum());
        assert(rR != this->model.size());

        SymbolType symbol = rR;

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
        const auto oL = this->L;
        const auto oR = this->R;
        const auto oD = this->D;
        const auto target = static_cast<double>(value - this->L) / this->D * this->model.sum();
        const auto oSum = this->model.sum();
#endif

        CounterType vL = this->model.asum(rR - 1U);
        CounterType vR = this->model.asum(rR);

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
        const double __r1 = static_cast<double>(vR) / this->model.sum();
        const double __r2 = static_cast<double>(vL) / this->model.sum();
        const double __rv = static_cast<double>(value - oL) / oD;
        const double __v1 = __r1 * this->D;
        const double __v2 = __r2 * this->D;
#endif

        this->R = this->L + static_cast<double>(vR) / this->model.sum() * this->D;
        this->L = this->L + static_cast<double>(vL) / this->model.sum() * this->D;
        this->D = this->R - this->L;

#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
        spdlog::info("[Output] symbol:[{:10d}] value:[{:10d}][L={:10d} ~ R={:10d}] "
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

            bool bit = this->device.get();
            if (this->device.eof()) bit = false;
            value = bits::append_bit(value, bit);
#ifdef CPPARMC_DEBUG_ARITHMETIC_DECODER
            spdlog::info("[Rerange] value=[{:d}][L={:d} ~ R={:d}] ov=[{:d}][oL={:d} ~ oR={:d}]",
                    value, this->L, this->R, ov, oov, ooL, ooR);
#endif
            assert((this->L <= value) && (value < this->R));
        }

        this->update_model(symbol);
        output_count += 1U;
        return symbol;
    }
}

#endif //CPPARMC_ARITHMETIC_DECODE_HPP
