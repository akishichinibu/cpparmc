#ifndef CPPARMC_BIT_STREAM_HPP
#define CPPARMC_BIT_STREAM_HPP

#include <cstdio>
#include <cassert>
#include <memory>

#include "__impl/stream/stream_base.hpp"
#include "__impl/stream/file_device.hpp"
#include "__impl/utils/bit_operation.hpp"


namespace cpparmc::stream {

    template<typename Device>
    class BitStream : public InputStream<Device> {

    private:
        u_int64_t ch;
        u_int64_t buffer;
        u_char buf_len;

    public:
        BitStream(Device& device, u_char output_width) :
                InputStream<Device>(device, device.output_width, output_width),
                ch(0U),
                buffer(0U),
                buf_len(0U) {}

        u_int64_t get() final {
            while (buf_len < this->output_width) {
                ch = this->device.get();
                if (this->device.eof()) break;

                buffer = bits::append_bits(buffer, ch, this->input_width);
                buf_len += this->input_width;
#ifdef CPPARMC_DEBUG_BIT_STREAM
                spdlog::debug("[bit_stream {:d}->{:d}] ch:[{:10d}] buffer:[{:10d}] buf_len:[{:10d}] eof:[{:3d}]",
                              this->input_width, this->output_width, ch, buffer, buf_len, this->device.eof());
#endif
            }

            if (buf_len == 0) {
                this->_eof = true;
                return EOF;
            }

            u_int64_t c;
            std::tie(c, buf_len) = bits::pop_bits(buffer, buf_len, this->output_width);
            return c;
        }
    };
}

#endif //CPPARMC_BIT_STREAM_HPP
