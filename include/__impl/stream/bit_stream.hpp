#ifndef CPPARMC_BIT_STREAM_HPP
#define CPPARMC_BIT_STREAM_HPP

#include <cstdio>
#include <cassert>
#include <memory>

#include "__impl/stream/stream_base.hpp"
#include "__impl/stream/file/read.hpp"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::stream {

    template<typename Device>
    class BitStream: public Stream<Device> {

    private:
        std::int64_t ch{};

    public:
        BitStream(Device& device, std::uint8_t output_width, bool greedy);

        BitStream() = default;

        StreamStatus receive() final;

        void set_output_width(std::uint8_t width);
    };

    template<typename Device>
    BitStream<Device>::BitStream(Device& device, std::uint8_t output_width, bool greedy) :
            Stream<Device>(device, device.output_width, output_width, greedy),
            ch(0) {};

    template<typename Device>
    auto BitStream<Device>::receive() -> StreamStatus {
        ch = this->device.get();
        return {this->device.eof() ? -1 : this->input_width, ch};
    }

    template<typename Device>
    void BitStream<Device>::set_output_width(std::uint8_t width) {
        this->output_count = this->output_count * this->output_width / width + this->greedy;
        this->output_width = width;
        this->_symbol_limit = 1U << this->output_width;
    }
}

#endif //CPPARMC_BIT_STREAM_HPP
