#ifndef CPPARMC_STREAM_FILE_WRITE_HPP
#define CPPARMC_STREAM_FILE_WRITE_HPP

#include <string>

#include "__impl/stream/file/base.hpp"


namespace cpparmc::stream {

    template<std::uint64_t max_buffer_size = 64 * 1024>
    class OutputFileDevice: public FileDeviceBase {

        char buffer[max_buffer_size]{};
        std::int64_t cursor;
        std::int64_t count;

    public:
        std::uint8_t output_width = 8;

        explicit OutputFileDevice(const std::string& fn):
        FileDeviceBase(fn),
        cursor(0),
        count(0) {
            this->open("wb");
            std::setvbuf(this->file, nullptr, _IOFBF, max_buffer_size);
        }

        void put(std::uint8_t c) {
            if (cursor == max_buffer_size) this->flush();
            buffer[cursor] = c;
            cursor += 1;
            count += 1;
        }

        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        void write(T val) {
            std::size_t len = sizeof(T);
            const auto rest = max_buffer_size - cursor;
            if (len > rest) this->flush();

            auto bit_len = len << 3U;
            while (bit_len > 0U) {
                std::tie(buffer[cursor], bit_len) = bits::pop_bits(val, bit_len, sizeof(char) << 3U);
                cursor += 1;
                count += 1;
            }
        }

        void flush() {
            if (cursor > 0) {
                std::fwrite(buffer, sizeof(char), cursor, this->file);
                this->check();
                std::fflush(this->file);
                this->check();
                cursor = 0;
            }
        }

        ~OutputFileDevice() {
            this->flush();
        }
    };
}

#endif //CPPARMC_STREAM_FILE_WRITE_HPP
