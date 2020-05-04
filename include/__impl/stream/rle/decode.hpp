#ifndef CPPARMC_RLE_DECODE_HPP
#define CPPARMC_RLE_DECODE_HPP

namespace cpparmc::stream {

    template<typename Device, typename SizeType=std::uint64_t>
    class RLEDecode: public Stream<Device> {

        std::int64_t previous_symbol;
        SizeType repeat_count;

        std::uint8_t symbol_bit;
        std::uint8_t counter_width;
        SizeType counter_limit;

        BitStream<Device> bit_adaptor;

        bool read_head;

    public:
        explicit RLEDecode(Device& device);
        StreamStatus receive() final;
    };

    template<typename Device, typename SizeType>
    RLEDecode<Device, SizeType>
    ::RLEDecode(Device& device):
            Stream<Device>(device, 8, 8, false),
            previous_symbol(EOF),
            repeat_count(0),
            symbol_bit(0),
            counter_width(0),
            counter_limit(0),
            bit_adaptor(device, 0, false),
            read_head(false) {};

    template<typename Device, typename SizeType>
    auto RLEDecode<Device, SizeType>
    ::receive() -> StreamStatus {
        if (!read_head) {
            read_head = true;

            std::uint16_t buf;
            const auto size = this->device.read(buf);
            assert(size == 2);

            symbol_bit = buf >> 8U;
            counter_width = buf & bits::get_n_repeat_bit(true, 8U);
            counter_limit = 1U << counter_width;

            bit_adaptor.set_output_width(symbol_bit + counter_width);

            DEBUG_PRINT("read a block with symbol_bit: {:d}, symbol_bit: {:d}. ", symbol_bit, counter_width);
            return { 0, 0 };
        }

        if (previous_symbol != EOF) {
            if (repeat_count == 0) {
                previous_symbol = EOF;
                return { 0, 0 };
            } else {
                repeat_count -= 1;
                return { symbol_bit, previous_symbol };
            }
        }

        repeat_count = bit_adaptor.get();
        if (bit_adaptor.eof()) return {-1, 0 };

        previous_symbol = std::get<0>(bits::pop_bits(repeat_count, symbol_bit + counter_width, symbol_bit));
        return { symbol_bit, previous_symbol };
    }
}

#endif //CPPARMC_RLE_DECODE_HPP
