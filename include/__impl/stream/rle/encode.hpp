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

    template<typename Device, typename SizeType=std::uint32_t>
    class RLEEncode: public InputStream<Device> {

        std::int64_t previous_symbol;
        SizeType count;

        std::uint8_t counter_width;
        SizeType counter_limit;
        bool has_read;

    public:
        RLEEncode(Device& device, std::uint8_t counter_width);
        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SizeType>
    RLEEncode<Device, SizeType>
    ::RLEEncode(Device& device, std::uint8_t counter_width):
    InputStream<Device>(device, device.output_width, device.output_width + counter_width),
    previous_symbol(EOF),
    count(0U),
    counter_width(counter_width),
    counter_limit(1U << counter_width),
    has_read(false) {
        previous_symbol = this->device.get();

        if (this->device.eof()) {
            this->_eof = true;
        }
    };

    template<typename Device, typename SizeType>
    auto RLEEncode<Device, SizeType>::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        while (true) {
            const auto ch = this->device.get();

            if (this->device.eof()) {
                this->_eof = true;
                bits::concat_bits(previous_symbol, count, counter_width);
                return { count > 0U ? this->output_width : 0, previous_symbol };
            }

            if ((ch == previous_symbol) && (count < counter_limit - 1U)) {
                count += 1U;
            } else {
                bits::concat_bits(previous_symbol, count, counter_width);
                const std::pair<std::uint8_t, std::uint64_t> result = { this->output_width, previous_symbol };
                count = 0U;
                previous_symbol = ch;
                return result;
            }
        }
    }
}

#endif //CPPARMC_RLE_ENCODE_HPP
