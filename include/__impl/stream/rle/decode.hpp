#ifndef CPPARMC_RLE_DECODE_HPP
#define CPPARMC_RLE_DECODE_HPP

namespace cpparmc::stream {

    template<typename Device, typename SizeType=std::uint64_t>
    class RLEDecode: public InputStream<Device> {

        std::int64_t previous_symbol;
        SizeType count;
        std::uint8_t counter_width;
        SizeType counter_limit;

    public:
        RLEDecode(Device& device, std::uint8_t counter_width);

        StreamStatus receive() final;
    };

    template<typename Device, typename SizeType>
    RLEDecode<Device, SizeType>
    ::RLEDecode(Device& device, std::uint8_t counter_width):
            InputStream<Device>(device, device.output_width, device.output_width - counter_width),
            previous_symbol(EOF),
            count(0),
            counter_width(counter_width),
            counter_limit(1U << counter_width) {};

    template<typename Device, typename SizeType>
    auto RLEDecode<Device, SizeType>
    ::receive() -> StreamStatus {
        if (previous_symbol == EOF) {
            count = this->device.get();
            if (this->device.eof()) return {-1, 0};
            previous_symbol = std::get<0>(bits::pop_bits(count, this->input_width, this->output_width));
            assert((0 <= count) && (count < counter_limit));
            return {this->output_width, previous_symbol};
        }

        if (count == 0) {
            previous_symbol = EOF;
            return {0, 0};
        } else {
            count -= 1;
            return {this->output_width, previous_symbol};
        }
    }
}

#endif //CPPARMC_RLE_DECODE_HPP
