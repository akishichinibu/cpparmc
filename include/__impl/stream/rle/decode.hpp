#ifndef CPPARMC_RLE_DECODE_HPP
#define CPPARMC_RLE_DECODE_HPP

namespace cpparmc::stream {

    template<typename Device, typename SizeType=std::uint32_t>
    class RLEDecode : public InputStream<Device> {

        std::int64_t previous_symbol;
        SizeType count;
        std::uint8_t counter_width;
        SizeType counter_limit;

    public:
        RLEDecode(Device& device, std::uint8_t counter_width);

        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SizeType>
    RLEDecode<Device, SizeType>
    ::RLEDecode(Device& device, std::uint8_t counter_width):
            InputStream<Device>(device, device.output_width, device.output_width - counter_width),
            previous_symbol(EOF),
            count(0U),
            counter_width(counter_width),
            counter_limit(1U << counter_width) {};

    template<typename Device, typename SizeType>
    auto RLEDecode<Device, SizeType>
    ::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        if ((previous_symbol == EOF) or (count == 0U)) {
            count = this->device.get();

            if (this->device.eof()) {
                this->_eof = true;
                return { 0U, 0U };
            }

            previous_symbol = std::get<0>(bits::pop_bits(count, this->input_width, this->output_width));
        }

        count -= 1U;
        return { this->output_width, previous_symbol };
    }
}

#endif //CPPARMC_RLE_DECODE_HPP
