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

        std::pair<std::int16_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SizeType>
    RLEEncode<Device, SizeType>
    ::RLEEncode(Device& device, std::uint8_t counter_width):
            InputStream<Device>(device, device.output_width, device.output_width + counter_width),
            previous_symbol(EOF),
            count(0),
            counter_width(counter_width),
            counter_limit(1U << counter_width),
            has_read(false) {};

    template<typename Device, typename SizeType>
    auto RLEEncode<Device, SizeType>::receive() -> StreamStatus {
        const auto ch = this->device.get();

        if (this->device.eof()) {
            if (previous_symbol == EOF) return {-1, 0};

            bits::concat_bits(previous_symbol, count, counter_width);
            const auto temp = previous_symbol;
            previous_symbol = EOF;
            return {this->output_width, temp};
        }

        if ((ch == previous_symbol) && (count < counter_limit - 1)) {
            count += 1;
            return {0, 0};
        } else {
            if (previous_symbol == EOF) {
                previous_symbol = ch;
                count = 0;
                return {0, 0};
            } else {
                bits::concat_bits(previous_symbol, count, counter_width);
                const StreamStatus r = {this->output_width, previous_symbol};
                count = 0;
                previous_symbol = ch;
                return r;
            }
        }
    }
}

#endif //CPPARMC_RLE_ENCODE_HPP
