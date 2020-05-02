#ifndef CPPARMC_STREAM_FILE_READ_HPP
#define CPPARMC_STREAM_FILE_READ_HPP

#include <string>
#include <tuple>

#include "__impl/stream/file/base.hpp"
#include "__impl/stream/stream_base.hpp"


namespace cpparmc::stream {

    template<std::uint64_t max_buffer_size = 4 * 1024 * 1024>
    class InputFileDevice: public FileDeviceBase, public InputStream<BaseStream> {
        char buffer[max_buffer_size]{};
        std::uint64_t cursor;
        std::uint64_t buffer_len;
        std::uint64_t count;

    public:
        explicit InputFileDevice(const std::string& fn):
        FileDeviceBase(fn),
        InputStream<BaseStream>(*this, 0, 8),
        cursor(0),
        buffer_len(0),
        count(0) {
            this->open("rb");
            std::setvbuf(this->file, nullptr, _IONBF, max_buffer_size);
        }

        [[nodiscard]] auto receive() -> StreamStatus final {
            if (cursor == buffer_len) {
                buffer_len = std::fread(buffer, sizeof(char), max_buffer_size, this->file);
                cursor = 0;
            }

            if (cursor == buffer_len) {
                return { -1, 0 };
            }

            count += 1;
            const std::uint8_t ch = buffer[cursor++];
            assert((0 <= ch < 256));
            return { this->output_width, ch };
        }

        [[nodiscard]] std::uint64_t tell() const {
            return count;
        }

        void reset() final {
            std::fseek(file, 0, SEEK_SET);
            count = 0;
            cursor = 0;
            buffer_len = 0;
            InputStream<BaseStream>::reset();
        }
    };
}

#endif //CPPARMC_STREAM_FILE_READ_HPP
