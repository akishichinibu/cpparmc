#ifndef CPPARMC_STDIN_HPP
#define CPPARMC_STDIN_HPP

#include "stream/io/buffer_io.hpp"

namespace cpparmc::stream {

    template<std::size_t max_buffer_size = 128 * 1024>
    class StdInputStream: public BufferIOMixin<max_buffer_size> {

    public:
        explicit StdInputStream() noexcept;

        std::size_t fill() noexcept final;
    };

    template<std::size_t mb>
    StdInputStream<mb>::StdInputStream() noexcept: BufferIOMixin<mb>() {}

    template<std::size_t mb>
    std::size_t StdInputStream<mb>::fill() noexcept {
        std::cin.read(this->buffer, mb);
        return std::cin.gcount();
    }
}

#endif //CPPARMC_STDIN_HPP
