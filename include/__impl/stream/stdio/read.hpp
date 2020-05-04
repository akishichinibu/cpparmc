#ifndef CPPARMC_STREAM_STDIO_READ_HPP
#define CPPARMC_STREAM_STDIO_READ_HPP

#include <string>
#include <iostream>
#include <tuple>

#include "__impl/stream/file/base.hpp"
#include "__impl/stream/stream_base.hpp"


namespace cpparmc::stream {

    template<typename SizeType, SizeType max_buffer_size>
    class IOStreamMixin: public Stream<BaseStream> {

    protected:
        char buffer[max_buffer_size] {};
        static_assert(sizeof(buffer) <= consts::allow_max_buffer_size);

        SizeType cursor;
        SizeType buffer_len;
        SizeType count;

        virtual SizeType read_into_buffer() = 0;

    public:
        IOStreamMixin():
                Stream<BaseStream>(*this, 8, 8),
                cursor(0),
                buffer_len(0),
                count(0) {}

        [[nodiscard]] auto receive() -> StreamStatus final {
            if (cursor == buffer_len) {
                buffer_len = this->read_into_buffer();
                cursor = 0;
            }

            if (cursor == buffer_len) return {-1, 0};

            const std::uint8_t ch = buffer[cursor];
            cursor += 1;
            count += 1;
            return { this->output_width, ch };
        }

        virtual SizeType tell() const = 0;

        void reset() override {
            count = 0;
            cursor = 0;
            buffer_len = 0;
            Stream<BaseStream>::reset();
        }
    };

    template<typename SizeType=consts::DefaultSizeType, std::uint64_t max_buffer_size = 128 * 1024>
    class StandardIOStream: public IOStreamMixin<SizeType, max_buffer_size> {

    public:
        explicit StandardIOStream();

        [[nodiscard]] std::uint64_t tell() const;

        std::uint64_t read_into_buffer() final;

        void reset() final;
    };

    template<typename SizeType, std::uint64_t max_buffer_size>
    StandardIOStream<SizeType, max_buffer_size>::StandardIOStream():
            IOStreamMixin<SizeType, max_buffer_size>() {}

    template<typename SizeType, std::uint64_t max_buffer_size>
    std::uint64_t StandardIOStream<SizeType, max_buffer_size>::read_into_buffer() {
        std::cin.read(this->buffer, max_buffer_size);
        return std::cin.gcount();
    }

    template<typename SizeType, std::uint64_t max_buffer_size>
    std::uint64_t StandardIOStream<SizeType, max_buffer_size>::tell() const {
        return std::cin.tellg();
    }

    template<typename SizeType, std::uint64_t max_buffer_size>
    void StandardIOStream<SizeType, max_buffer_size>::reset() {
        std::cin.seekg(SEEK_SET);
        IOStreamMixin<SizeType, max_buffer_size>::reset();
    }
}

#endif //CPPARMC_STREAM_STDIO_READ_HPP
