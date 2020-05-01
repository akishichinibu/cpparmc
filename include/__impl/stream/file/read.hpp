#ifndef CPPARMC_STREAM_FILE_READ_HPP
#define CPPARMC_STREAM_FILE_READ_HPP

#include <string>

#include "__impl/stream/file/base.hpp"
#include "__impl/stream/stream_base.hpp"


namespace cpparmc::stream {

    template<u_int64_t buffer_size=64 * 1024>
    class InputFileDevice : public FileDeviceBase, public InputStream<BaseStream> {

    public:
        explicit InputFileDevice(const std::string& fn) :
                FileDeviceBase(fn),
                InputStream<BaseStream>(*this, 8U, 8U) {
            this->open("rb");
            FileDeviceBase::check(std::setvbuf(this->file, nullptr, _IOFBF, buffer_size));
        }

        [[nodiscard]] auto receive() -> std::pair<std::uint8_t, std::uint64_t> final {
            const auto ch = std::fgetc(this->file);
            this->_eof = std::feof(this->file);
            return { this->output_width, ch };
        }

        [[nodiscard]] u_long tell() const {
            return std::ftell(this->file);
        }

        template<typename T>
        InputFileDevice& read(T& val) {
            val = 0U;
            for (auto i = 0; i < sizeof(T); i++) {
                val = (val << 8U) | fgetc(this->file);
            }
            return *this;
        }

        void reset() final {
            std::fseek(file, 0, SEEK_SET);
            _eof = false;
        }
    };

}

#endif //CPPARMC_STREAM_FILE_READ_HPP
