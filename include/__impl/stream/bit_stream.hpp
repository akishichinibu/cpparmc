#ifndef CPPARMC_BIT_STREAM_HPP
#define CPPARMC_BIT_STREAM_HPP

#include <cstdio>
#include <cassert>
#include <memory>

#include "__impl/stream/stream_base.hpp"
#include "__impl/stream/file_device.hpp"


namespace cpparmc {

    template<typename Device>
    class BitStream: public InputStream<Device> {

    private:
        u_int32_t ch;
        u_int32_t buffer;
        u_char buffer_length;

    public:
        BitStream(Device& device, u_char output_width):
                InputStream<Device>(device, device.output_width, output_width),
                ch(0U),
                buffer(0U),
                buffer_length(0U) {}

        u_int32_t get() final {
            while (buffer_length < this->output_width) {
                ch = this->device.get();
                if (this->device.eof()) break;

                buffer = (buffer << this->input_width) | ch;
                buffer_length += this->input_width;
#ifdef BIT_STREAM_DEBUG
                printf("[bit_stream %d:%d] %10d %10d %10d %10d\n",
                        this->input_width, this->output_width, ch, buffer, buffer_length, this->device.eof());
#endif
            }

            if (buffer_length == 0) {
                this->_eof = true;
                return EOF;
            }

            if (buffer_length <= this->output_width) {
                const auto rest_width = this->output_width - buffer_length;
                const auto c = buffer << rest_width;
                buffer = 0U;
                buffer_length = 0U;
                return c;
            } else {
                const auto rest_width = buffer_length - this->output_width;
                const auto c = buffer >> rest_width;
                buffer &= (1U << rest_width) - 1U;
                buffer_length = rest_width;
                return c;
            }
        }
    };
}

#endif //CPPARMC_BIT_STREAM_HPP
