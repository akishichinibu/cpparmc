#ifndef CPPARMC_RLE_ENCODE_HPP
#define CPPARMC_RLE_ENCODE_HPP

#include <vector>
#include <string>
#include <string_view>
#include <numeric>

#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/timer.hpp"
#include "__impl/darray.hpp"


namespace cpparmc::stream {

    template<typename Device, typename SymbolType=u_char, typename SizeType=std::uint32_t>
    class RLEEncode: public InputStream<Device> {

        SymbolType previous_symbol;
        SizeType count;

        std::uint8_t counter_width;
        SizeType counter_limit;
        bool has_read;

    public:
        RLEEncode(Device& device, std::uint8_t counter_width);
        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SymbolType, typename SizeType>
    RLEEncode<Device, SymbolType, SizeType>
    ::RLEEncode(Device& device, std::uint8_t counter_width):
    InputStream<Device>(device, device.output_width, device.output_width + counter_width),
    previous_symbol(0U),
    count(0U),
    counter_width(counter_width),
    counter_limit(1U << counter_width),
    has_read(false) {
        previous_symbol = this->device.get();

        if (this->device.eof()) {
            this->_eof = true;
        }
    };

    template<typename Device, typename SymbolType, typename SizeType>
    auto RLEEncode<Device, SymbolType, SizeType>::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        while (true) {
            const auto ch = this->device.get();

            if (this->device.eof()) {
                this->_eof = true;
                return {
                        count > 0U ? this->output_width : 0,
                        bits::append_bits(previous_symbol, count, counter_width)
                };
            }

            if ((ch == previous_symbol) && (count < counter_limit - 1U)) {
                count += 1U;
            } else {
                const std::pair<std::uint8_t, std::uint64_t> result = {
                        this->output_width,
                        bits::append_bits(previous_symbol, count, counter_width)
                };
                count = 0U;
                previous_symbol = ch;
                return result;
            }
        }
    }
}

#endif //CPPARMC_RLE_ENCODE_HPP
