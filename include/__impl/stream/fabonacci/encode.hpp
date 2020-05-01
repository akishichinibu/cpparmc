#ifndef CPPARMC_FABONACCI_ENCODE_HPP
#define CPPARMC_FABONACCI_ENCODE_HPP

#include "__impl/stream/stream_base.hpp"
#include "fibonacci_code.h"

namespace cpparmc::stream {

    template<typename Device>
    class FibonacciEncode : public InputStream<Device> {

    protected:
        std::int64_t ch;
        std::uint64_t buffer;
        std::uint8_t buffer_length;

    public:
        FibonacciEncode(Device& device, u_int8_t output_width);

        std::int64_t get() override;
    };

    template<typename Device>
    FibonacciEncode<Device>::
    FibonacciEncode(Device& device, u_int8_t output_width) :
            InputStream<Device>(device, device.output_width, output_width),
            ch(0U),
            buffer(0U),
            buffer_length(0U) {}

    template<typename Device>
    std::int64_t FibonacciEncode<Device>::get() {
        while (buffer_length < this->output_width) {
            ch = this->device.get();
            if (this->device.eof()) break;

            u_char length;
            std::uint64_t value;
            std::tie(length, value) = consts::fibonacci_code[ch];

            buffer = (buffer << length) | value;
            buffer_length += length;
        }

        if (buffer_length == 0U) {
            this->_eof = true;
            return EOF;
        }

        const auto rest_len = buffer_length - this->output_width;
        const std::uint64_t result = buffer >> rest_len;
        buffer &= (1U << rest_len) - 1U;
        buffer_length = rest_len;

        return result;
    }


    template<typename Device>
    class ConditionalFibonacciEncode : public FibonacciEncode<Device> {

        std::uint64_t total_symbol;
        darray<std::uint64_t> stat;
        bool has_stat;
        bool has_output_header;
        bool encode_deci;

    public:

            ConditionalFibonacciEncode(Device& device, u_int8_t output_width);

        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device>
    ConditionalFibonacciEncode<Device>
            ::ConditionalFibonacciEncode(Device& device, u_int8_t output_width):
            FibonacciEncode<Device>(device, output_width),
            total_symbol(1U << device.output_width),
            stat(total_symbol),
            has_stat(false),
            has_output_header(false),
            encode_deci(false) {};

    template<typename Device>
    auto ConditionalFibonacciEncode<Device>::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        if (!has_stat) {
            std::uint64_t count = 0U;

            while (true) {
                this->ch = this->device.get();
                if (this->device.eof()) break;
                stat[this->ch] += 1U;
                count += this->device.output_width;
            }

            std::sort(stat.begin(), stat.end(), std::greater<>());

            std::uint64_t length = 0U;

            for (auto i = 0; i < total_symbol; i++) {
                length += stat[i] * std::get<0>(consts::fibonacci_code[i]);
            }

#ifdef CPPARMC_DEBUG_FIBONACCI_ENCODE
            spdlog::info("The stream has length=[{:d}] bit and fibonacci=[{:d}] bit. ",
                    count, length);

            for (auto i = 0; i < total_symbol; i++) {
                printf("%lu   ", stat[i]);
                if ((i + 1) % 10 == 0) printf("\n");
            }

            printf("\n");
#endif
            has_stat = true;
            encode_deci = length < count;
            this->reset();
        }

        if (!has_output_header) {
            has_output_header = true;
            return { this->output_width, encode_deci ? 0b00001111 : 0b11110000 };
        }

        if (encode_deci) {
            return { this->output_width, FibonacciEncode<Device>::get() };
        } else {
            this->ch = this->device.get();
            if (this->device.eof()) this->_eof = true;
            return { this->output_width, this->ch };
        }
    }
}


#endif //CPPARMC_FABONACCI_ENCODE_HPP
