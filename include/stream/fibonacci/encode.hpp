#ifndef CPPARMC_FIBONACCI_ENCODE_HPP
#define CPPARMC_FIBONACCI_ENCODE_HPP

#include "stream/generator.hpp"
#include "stream/fibonacci/fibonacci_code.h"

namespace cpparmc::stream {

    using namespace utils;
    using namespace consts;

    template<typename Device>
    class FibonacciEncode: public Generator<Device> {

    public:
        explicit FibonacciEncode(Device& device) noexcept;

        StreamStatus patch() noexcept final;
    };

    template<typename Device>
    FibonacciEncode<Device>::FibonacciEncode(Device& device) noexcept
    :Generator<Device>(device) {}

    template<typename Device>
    StreamStatus FibonacciEncode<Device>::patch() noexcept {
        const auto buf = this->src.next(8U);

        if (!buf) {
            return std::nullopt;
        }

        StreamSizeType width;
        SymbolType symbol;
        std::tie(width, symbol) = buf.value();

        const auto r = consts::fibonacci_code[symbol];
        return StreamStatus(std::in_place, std::get<0>(r), std::get<1>(r));
    }
}

#endif //CPPARMC_FIBONACCI_ENCODE_HPP
