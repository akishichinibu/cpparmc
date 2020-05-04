#ifndef CPPARMC_RLE_ENCODE_HPP
#define CPPARMC_RLE_ENCODE_HPP

#include <vector>
#include <string>
#include <string_view>
#include <numeric>

#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/timer.hpp"
#include "__impl/utils/darray.hpp"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::stream {

    template<typename Device, typename SizeType=std::uint64_t>
    class RLEEncode: public Stream<Device> {

        std::int64_t previous_symbol;
        SizeType repeat_count;

        std::uint8_t symbol_bit;
        std::uint8_t counter_width;
        SizeType counter_limit;
        BitStream<Device> bit_input_adaptor;

        bool send_head;

    public:
        RLEEncode(Device& device, std::uint8_t symbol_bit, std::uint8_t counter_width);
        std::pair<std::int16_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SizeType>
    RLEEncode<Device, SizeType>
    ::RLEEncode(Device& device, std::uint8_t symbol_bit, std::uint8_t counter_width):
            Stream<Device>(device, 8, 8, true),
            previous_symbol(EOF),
            repeat_count(0),
            symbol_bit(symbol_bit),
            counter_width(counter_width),
            counter_limit(1U << counter_width),
            bit_input_adaptor(BitStream<Device>(device, symbol_bit, true)),
            send_head(false) {};

    template<typename Device, typename SizeType>
    auto RLEEncode<Device, SizeType>::receive() -> StreamStatus {
        if (!send_head) {
            send_head = true;
            DEBUG_PRINT("read a block with symbol_bit: {:d}, symbol_bit: {:d}. ", symbol_bit, counter_width);
            return { 16, (symbol_bit << 8U) | counter_width };
        }

        const auto ch = bit_input_adaptor.get();

        if (bit_input_adaptor.eof()) {
            if (previous_symbol == EOF) return {-1, 0};

            bits::concat_bits(previous_symbol, repeat_count, counter_width);
            const StreamStatus r = { symbol_bit + counter_width, previous_symbol };
            previous_symbol = EOF;
            return r;
        }

        if ((ch == previous_symbol) && (repeat_count < counter_limit - 1)) {
            repeat_count += 1;
            return { 0, 0 };
        }

        if (previous_symbol == EOF) {
            previous_symbol = ch;
            repeat_count = 0;
            return { 0, 0 };
        }

        bits::concat_bits(previous_symbol, repeat_count, counter_width);
        const StreamStatus r = { symbol_bit + counter_width, previous_symbol };
        repeat_count = 0;
        previous_symbol = ch;
        return r;
    }
}

#endif //CPPARMC_RLE_ENCODE_HPP
