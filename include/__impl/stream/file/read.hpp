#ifndef CPPARMC_STREAM_FILE_READ_HPP
#define CPPARMC_STREAM_FILE_READ_HPP

#include <string>
#include <tuple>

#include "__impl/stream/file/base.hpp"
#include "__impl/stream/stream_base.hpp"


namespace cpparmc::stream {

    template<u_int64_t buffer_size = 64 * 1024>
    class InputFileDevice: public FileDeviceBase, public InputStream<BaseStream> {

    public:
        explicit InputFileDevice(const std::string& fn):
                FileDeviceBase(fn),
                InputStream<BaseStream>(*this, 0, 8) {
            this->open("rb");
        }

        [[nodiscard]] auto receive() -> StreamStatus final {
            const auto ch = std::fgetc(this->file);
            const bool eof = std::feof(this->file);
            return {eof ? -1 : this->output_width, ch};
        }

        [[nodiscard]] u_long tell() const {
            return std::ftell(this->file);
        }

        void reset() final {
            std::fseek(file, 0, SEEK_SET);
            InputStream<BaseStream>::reset();
        }
    };

}

#endif //CPPARMC_STREAM_FILE_READ_HPP
