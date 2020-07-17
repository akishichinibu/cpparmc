#ifndef CPPARMC_RLE_ENCODE_HPP
#define CPPARMC_RLE_ENCODE_HPP

#include <vector>
#include <string>
#include <string_view>
#include <numeric>

#include "stream/generator.hpp"
#include "utils/timer.hpp"
#include "utils/darray.hpp"
#include "utils/bit_operation.hpp"


namespace cpparmc::stream {

    template<typename Device>
    class RLEEncode: public Generator<Device> {
        typedef Generator <Device> generator_type;

        SymbolType previous_symbol;
        std::uint64_t repeat_count;

        StreamSizeType symbol_bit;
        std::uint8_t counter_width;

        std::uint64_t counter_limit;

    public:
        RLEEncode(Device& device, std::uint8_t symbol_bit, std::uint8_t counter_width) noexcept;

        StreamStatus patch() noexcept final;
    };

    template<typename Device>
    RLEEncode<Device>
    ::RLEEncode(Device& device, std::uint8_t symbol_bit, std::uint8_t counter_width) noexcept:
            Generator<Device>(device),
            previous_symbol(EOF),
            repeat_count(0),
            symbol_bit(symbol_bit),
            counter_width(counter_width),
            counter_limit(1U << counter_width) {
        DEBUG_PRINT("read a block with symbol_bit: {:d}, symbol_bit: {:d}. ", symbol_bit, counter_width);
        this->send(8, symbol_bit);
        this->send(8, counter_width);
    }

    template<typename Device>
    auto RLEEncode<Device>::patch() noexcept -> StreamStatus {
        const auto frame = this->src.next(symbol_bit);

        if (this->src_eof()) {
            if (previous_symbol == EOF) return std::nullopt;

            this->send(symbol_bit, previous_symbol);
            this->send(counter_width, repeat_count);

            previous_symbol = EOF;
            return empty_frame;
        }

        const auto ch = std::get<1>(frame.value());

        if ((ch == previous_symbol) && (repeat_count < counter_limit - 1)) {
            repeat_count += 1;
            return empty_frame;
        }

        if (previous_symbol == EOF) {
            previous_symbol = ch;
            repeat_count = 0;
            return empty_frame;
        }

        this->send(symbol_bit, previous_symbol);
        this->send(counter_width, repeat_count);

        repeat_count = 0;
        previous_symbol = ch;
        return empty_frame;
    }
}

#endif //CPPARMC_RLE_ENCODE_HPP
