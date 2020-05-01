#ifndef CPPARMC_STREAM_FILE_WRITE_HPP
#define CPPARMC_STREAM_FILE_WRITE_HPP

#include <string>

#include "__impl/stream/file/base.hpp"


namespace cpparmc::stream {

    class OutputFileDevice: public FileDeviceBase {

    public:
        u_char output_width = 8;

        explicit OutputFileDevice(const std::string& fn):
                FileDeviceBase(fn) {
            this->open("wb");
        }

        [[nodiscard]] OutputFileDevice& put(u_char c) {
            std::putc(c, this->file);
            this->check();
            return *this;
        }

        template<typename T>
        OutputFileDevice& write(T* val, std::size_t n) {
            std::fwrite(val, sizeof(T), n, this->file);
            this->check();
            return *this;
        }

        template<typename T>
        OutputFileDevice& write(T& val) {
            std::fwrite(&val, sizeof(T), 1, this->file);
            this->check();
            return *this;
        }
    };
}

#endif //CPPARMC_STREAM_FILE_WRITE_HPP
