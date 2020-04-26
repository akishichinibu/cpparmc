#ifndef CPPARMC_FABONACCI_ENCODE_HPP
#define CPPARMC_FABONACCI_ENCODE_HPP

#include "__impl/fibonacci_code.h"

namespace cpparmc {

    template<typename Device>
    class FabonacciEncode : public InputStream<Device> {

    private:
        u_int64_t ch;
        u_int64_t buffer;
        u_int8_t buffer_length;

    public:
        explicit FabonacciEncode(Device& device, u_int8_t output_width) :
                InputStream<Device>(device, device.input_width, output_width),
                ch(0U),
                buffer(0U),
                buffer_length(0U) {}

        u_int32_t get() final {
            while (buffer_length < this->output_width) {
                ch = this->device.get();
                if (this->device.eof()) break;

                u_char length;
                u_int32_t value;
                std::tie(length, value) = fibonacci_code[ch];

                buffer = (buffer << length) | value;
                buffer_length += length;
            }

            if (buffer_length == 0U) {
                this->_eof = true;
                return EOF;
            }

            const auto rest_len = buffer_length - this->output_width;
            const u_int32_t bit = buffer >> rest_len;
            buffer &= (1U << rest_len) - 1U;
            buffer_length = rest_len;

            return bit;
        }
    };
};

#endif //CPPARMC_FABONACCI_ENCODE_HPP
