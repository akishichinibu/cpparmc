#ifndef CPPARMC_RLE_DECODE_HPP
#define CPPARMC_RLE_DECODE_HPP

namespace cpparmc::stream {

    template<typename Device, typename SizeType=std::uint64_t>
    class RLEDecode: public Generator<Device> {
        typedef Generator <Device> generator_type;

        SymbolType previous_symbol;
        SizeType repeat_count;

        std::uint8_t symbol_bit;
        std::uint8_t counter_width;
        SizeType counter_limit;

    public:
        explicit RLEDecode(Device& src);

        StreamStatus patch() noexcept final;
    };

    template<typename Source, typename SizeType>
    RLEDecode<Source, SizeType>
    ::RLEDecode(Source& src):
            Generator<Source>(src),
            previous_symbol(EOF),
            repeat_count(0),
            symbol_bit([&]() {
                const auto frame = src.next(8);
                return std::get<1>(frame.value());
            }()),
            counter_width([&]() {
                const auto frame = src.next(8);
                return std::get<1>(frame.value());
            }()),
            counter_limit(1U << counter_width) {
        DEBUG_PRINT("read a block with symbol_bit: {:d}, symbol_bit: {:d}. ", symbol_bit, counter_width);
    };

    template<typename Device, typename SizeType>
    auto RLEDecode<Device, SizeType>
    ::patch() noexcept -> StreamStatus {
        if (previous_symbol != EOF) {
            if (repeat_count == 0) {
                previous_symbol = EOF;
            } else {
                repeat_count -= 1;
            }

            return StreamStatus(std::in_place, symbol_bit, previous_symbol);
        }

        const auto frame_symbol = this->src.next(symbol_bit);
        if (this->src_eof()) return std::nullopt;

        const auto frame_count = this->src.next(counter_width);
        if (this->src_eof()) return std::nullopt;

        previous_symbol = std::get<1>(frame_symbol.value());
        repeat_count = std::get<1>(frame_count.value());
        return empty_frame;
    }
}

#endif //CPPARMC_RLE_DECODE_HPP
