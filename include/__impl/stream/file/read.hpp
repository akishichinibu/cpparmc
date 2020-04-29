#ifndef CPPARMC_STREAM_FILE_READ_HPP
#define CPPARMC_STREAM_FILE_READ_HPP

#include <string>

#include "__impl/stream/file/base.hpp"
#include "__impl/stream/stream_base.hpp"


namespace cpparmc::stream {

    class InputFileDevice : public FileDeviceBase, public InputStream<FileDeviceBase> {

    public:
        explicit InputFileDevice(const std::string& fn) :
                FileDeviceBase(fn),
                InputStream<FileDeviceBase>(*this, 8U, 8U) {
            this->open("rb");
        }

        [[nodiscard]] std::int64_t get() {
            const auto ch = std::fgetc(this->file);
            this->_eof = std::feof(this->file);
            return ch;
        }

        template<typename T>
        InputFileDevice& read(T& val) {
            std::fread(&val, sizeof(T), 1, this->file);
            this->check();
            return *this;
        }

        [[nodiscard]] u_long tell() const {
            return std::ftell(this->file);
        }

        void reset() {
            std::fseek(file, 0, SEEK_SET);
            _eof = false;
        }
    };

}

#endif //CPPARMC_STREAM_FILE_READ_HPP
