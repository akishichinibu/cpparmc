#ifndef CPPARMC_BUFFER_IO_HPP
#define CPPARMC_BUFFER_IO_HPP

#include "stream/generator.hpp"


namespace cpparmc::stream {

    template<std::size_t max_buffer_size>
    class BufferIOMixin: public Generator<OutsideSource> {

    protected:
        static_assert(sizeof(max_buffer_size) <= 1 * 1024 * 1024);
        char buffer[max_buffer_size] {};

        std::size_t cursor;
        std::size_t buffer_len;

        virtual std::size_t fill() noexcept = 0;

    public:
        BufferIOMixin();

        StreamStatus patch() noexcept final;
    };

    template<std::size_t mb>
    BufferIOMixin<mb>::BufferIOMixin():
    Generator<OutsideSource>(*this),
    cursor(0),
    buffer_len(0) {}

    template<std::size_t mb>
    StreamStatus BufferIOMixin<mb>::patch() noexcept {
        if (cursor != buffer_len) {
            return StreamStatus(std::in_place, 8, buffer[cursor++]);
        }

        cursor = 0;
        buffer_len = 0;
        if (cursor == buffer_len) buffer_len = this->fill();
        return cursor == buffer_len ? std::nullopt : StreamStatus(std::in_place, 0, 0);
    }
}

#endif //CPPARMC_BUFFER_IO_HPP
