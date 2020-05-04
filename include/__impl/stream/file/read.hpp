#ifndef CPPARMC_STREAM_FILE_READ_HPP
#define CPPARMC_STREAM_FILE_READ_HPP

#include <string>
#include <tuple>

#include "__impl/stream/file/base.hpp"
#include "__impl/stream/stream_base.hpp"


namespace cpparmc::stream {

    template<std::size_t max_buffer_size = 64 * 1024>
    class InputFileDevice: public FileDeviceBase, public Stream<BaseStream> {

        char buffer[max_buffer_size]{};
        std::size_t cursor;
        std::size_t buffer_len;
        std::size_t count;

    public:
        explicit InputFileDevice(const std::string& fn):
                FileDeviceBase(fn),
                Stream<BaseStream>(*this, 8, 8),
                cursor(0),
                buffer_len(0),
                count(0) {
            this->open("rb");
            std::setvbuf(this->file, nullptr, _IONBF, max_buffer_size);
        }

        [[nodiscard]] auto receive() -> StreamStatus final {
            DEBUG_PRINT("FILE!!! {:d}  {:d}", buffer_len, cursor);

            if (cursor == buffer_len) {
                buffer_len = std::fread(buffer, sizeof(char), max_buffer_size, this->file);
                cursor = 0;
            }

            if (cursor == buffer_len) return { -1, 0 };

            const std::uint8_t ch = buffer[cursor];
            count += 1;
            cursor += 1;
            return { this->output_width, ch };
        }

        [[nodiscard]] std::size_t tell() const {
            return count;
        }

        void reset() final {
            std::fseek(file, 0, SEEK_SET);
            count = 0;
            cursor = 0;
            buffer_len = 0;
            Stream<BaseStream>::reset();
        }
    };
}

#endif //CPPARMC_STREAM_FILE_READ_HPP
