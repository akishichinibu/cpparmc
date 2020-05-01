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
    class BitStream : public InputStream<Device> {

    private:
        std::int64_t ch;

    public:
        BitStream(Device& device, u_char output_width);
        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device>
    BitStream<Device>::BitStream(Device& device, u_char output_width) :
            InputStream<Device>(device, device.output_width, output_width),
            ch(0U) {};

    template<typename Device>
    auto BitStream<Device>::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        ch = this->device.get();
        if (this->device.eof()) {
            return { 0, 0 };
        } else {
            return { this->output_width, ch };
        }
    }
}

#endif //CPPARMC_BIT_STREAM_HPP
